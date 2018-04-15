

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <expr.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <Pat_inst_ref.h>
#include <string/common.h>

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STACK_SIZE 32


typedef bool (*Op_func)(const Value* op1, const Value* op2, Value* res, Streader* sr);


typedef struct Operator
{
    const char* name;
    int preced;
    Op_func func;
} Operator;


static bool evaluate_expr_(
        Streader* sr,
        Env_state* estate,
        Value* val_stack,
        int vsi,
        Operator* op_stack,
        int osi,
        const Value* meta,
        Value* res,
        int depth,
        bool func_arg,
        Random* rand);


static bool handle_unary(Value* val, bool found_not, bool found_minus, Streader* sr);


static bool get_token(Streader* sr, char* result);
static bool get_str_token(Streader* sr, char* result);
static bool get_num_token(Streader* sr, char* result);
static bool get_var_token(Streader* sr, char* result);
static bool get_op_token(Streader* sr, char* result);


static bool Value_from_token(
        Value* val, char* token, Env_state* estate, const Value* meta);

//static void Value_print(Value* val);


#define OPERATOR_AUTO (&(Operator){ .name = NULL, .func = NULL })


static bool Operator_from_token(Operator* op, char* token);

//static void Operator_print(Operator* op);


#define OP_DECL(name) static bool name(const Value*, const Value*, Value*, Streader* sr)

OP_DECL(op_neq);
OP_DECL(op_leq);
OP_DECL(op_geq);
OP_DECL(op_or);
OP_DECL(op_and);
OP_DECL(op_eq);
OP_DECL(op_lt);
OP_DECL(op_gt);
OP_DECL(op_add);
OP_DECL(op_sub);
OP_DECL(op_mul);
OP_DECL(op_div);
OP_DECL(op_mod);
OP_DECL(op_pow);

#undef OP_DECL


static Operator operators[] =
{
    { .name = "!=", .preced = 2, .func = op_neq },
    { .name = "<=", .preced = 3, .func = op_leq },
    { .name = ">=", .preced = 3, .func = op_geq },
    { .name = "|",  .preced = 0, .func = op_or },
    { .name = "&",  .preced = 1, .func = op_and },
    { .name = "=",  .preced = 2, .func = op_eq },
    { .name = "!",  .preced = 3, .func = NULL },
    { .name = "<",  .preced = 3, .func = op_lt },
    { .name = ">",  .preced = 3, .func = op_gt },
    { .name = "+",  .preced = 4, .func = op_add },
    { .name = "-",  .preced = 4, .func = op_sub },
    { .name = "*",  .preced = 5, .func = op_mul },
    { .name = "/",  .preced = 5, .func = op_div },
    { .name = "%",  .preced = 5, .func = op_mod },
    { .name = "^",  .preced = 6, .func = op_pow },
    { .name = NULL,              .func = NULL }
};


typedef bool (*Func)(const Value* args, Value* res, Random* rand, Streader* sr);


#define FUNC_ARGS_MAX 4


static bool token_is_func(const char* token, Func* res);


typedef struct Func_desc
{
    const char* name;
    Func func;
} Func_desc;


#define FUNC_DECL(fn) static bool func_##fn( \
        const Value* args, Value* res, Random* rand, Streader* sr)

FUNC_DECL(ts);
FUNC_DECL(rand);
FUNC_DECL(pat);

#undef FUNC_DECL


static Func_desc funcs[] =
{
    { .name = "ts",   .func = func_ts },
    { .name = "rand", .func = func_rand },
    { .name = "pat",  .func = func_pat },
    { .name = NULL,   .func = NULL }
};


bool evaluate_expr(
        Streader* sr, Env_state* estate, const Value* meta, Value* res, Random* rand)
{
    rassert(sr != NULL);
    rassert(res != NULL);
    rassert(rand != NULL);

    if (!Streader_match_char(sr, '"'))
        return false;

    Value val_stack[STACK_SIZE] = { { .type = VALUE_TYPE_NONE } };
    Operator op_stack[STACK_SIZE] = { { .name = NULL } };

    if (meta == NULL)
        meta = VALUE_AUTO;

    return evaluate_expr_(
            sr,
            estate,
            val_stack,
            0,
            op_stack,
            0,
            meta,
            res,
            0,
            false,
            rand);
}


#define check_stack(si) if (true)                     \
    {                                                 \
        if ((si) >= STACK_SIZE)                       \
        {                                             \
            rassert((si) == STACK_SIZE);              \
            Streader_set_error(sr, "Stack overflow"); \
            return false;                             \
        }                                             \
    } else ignore(0)

static bool evaluate_expr_(
        Streader* sr,
        Env_state* estate,
        Value* val_stack,
        int vsi,
        Operator* op_stack,
        int osi,
        const Value* meta,
        Value* res,
        int depth,
        bool func_arg,
        Random* rand)
{
    rassert(sr != NULL);
    rassert(val_stack != NULL);
    rassert(vsi >= 0);
    rassert(vsi <= STACK_SIZE);
    rassert(op_stack != NULL);
    rassert(osi >= 0);
    rassert(osi <= STACK_SIZE);
    rassert(meta != NULL);
    rassert(res != NULL);
    rassert(depth >= 0);
    rassert(rand != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (depth >= STACK_SIZE)
    {
        Streader_set_error(sr, "Maximum recursion depth exceeded");
        return false;
    }

    int orig_vsi = vsi;
    int orig_osi = osi;
    char token[KQT_VAR_NAME_MAX + 1 + 4] = ""; // + 4 for delimiting \"s
    bool expect_operand = true;
    bool found_not = false;
    bool found_minus = false;

    int64_t prev_pos = sr->pos;
    while (get_token(sr, token) &&
            !string_eq(token, "") &&
            !string_eq(token, ")") &&
            (!func_arg || !string_eq(token, ",")))
    {
        Value* operand = VALUE_AUTO;
        Operator* op = OPERATOR_AUTO;
        Func func = NULL;

        if (string_eq(token, "("))
        {
            if (!expect_operand)
            {
                Streader_set_error(sr, "Unexpected operand");
                return false;
            }

            check_stack(vsi);
            if (!evaluate_expr_(
                        sr, estate, val_stack, vsi,
                        op_stack, osi, meta, operand, depth + 1,
                        false, rand))
                return false;

            rassert(operand->type != VALUE_TYPE_NONE);
            if (!handle_unary(operand, found_not, found_minus, sr))
                return false;

            found_not = found_minus = false;
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
        }
        else if (token_is_func(token, &func))
        {
            if (!expect_operand)
            {
                Streader_set_error(sr, "Unexpected function");
                return false;
            }

            check_stack(vsi);
            rassert(func != NULL);
            Value func_args[FUNC_ARGS_MAX] = { { .type = VALUE_TYPE_NONE } };
            if (!Streader_match_char(sr, '('))
                return false;

            int i = 0;
            if (!Streader_try_match_char(sr, ')'))
            {
                for (i = 0; i < FUNC_ARGS_MAX; ++i)
                {
                    if (!evaluate_expr_(
                                sr, estate, val_stack, vsi,
                                op_stack, osi, meta, &func_args[i], depth + 1,
                                true, rand))
                        return false;

                    rassert(func_args[i].type != VALUE_TYPE_NONE);
                    if (Streader_try_match_char(sr, ')'))
                    {
                        ++i;
                        break;
                    }

                    if (!Streader_match_char(sr, ','))
                        return false;
                }
            }

            if (i < FUNC_ARGS_MAX)
                func_args[i].type = VALUE_TYPE_NONE;

            if (!func(func_args, operand, rand, sr))
            {
                rassert(Streader_is_error_set(sr));
                return false;
            }

            rassert(operand->type != VALUE_TYPE_NONE);
            found_not = found_minus = false;
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
        }
        else if (Value_from_token(operand, token, estate, meta))
        {
            if (!expect_operand)
            {
                Streader_set_error(sr, "Unexpected operand");
                return false;
            }

            rassert(operand->type != VALUE_TYPE_NONE);
            if (!handle_unary(operand, found_not, found_minus, sr))
                return false;

            found_not = found_minus = false;
            check_stack(vsi);
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
            //Value_print(operand);
        }
        else if (Operator_from_token(op, token))
        {
            rassert(op->name != NULL);
            if (expect_operand)
            {
                if (string_eq(op->name, "!"))
                {
                    found_not = true;
                }
                else if (string_eq(op->name, "-"))
                {
                    found_minus = true;
                }
                else
                {
                    Streader_set_error(sr, "Unexpected binary operator");
                    return false;
                }

                prev_pos = sr->pos;
                continue;
            }

            if (string_eq(op->name, "!"))
            {
                Streader_set_error(sr, "Unexpected boolean not");
                return false;
            }

            while (osi > orig_osi && op->preced <= op_stack[osi - 1].preced)
            {
                Operator* top = &op_stack[osi - 1];
                rassert(top->name != NULL);
                rassert(top->func != NULL);

                if (vsi < 2)
                {
                    Streader_set_error(sr, "Not enough operands");
                    return false;
                }

                Value* result = VALUE_AUTO;
                if (!top->func(
                            &val_stack[vsi - 2],
                            &val_stack[vsi - 1],
                            result,
                            sr))
                {
                    rassert(Streader_is_error_set(sr));
                    return false;
                }

                //Operator_print(top);
                rassert(result->type != VALUE_TYPE_NONE);
                val_stack[vsi - 2].type = VALUE_TYPE_NONE;
                val_stack[vsi - 1].type = VALUE_TYPE_NONE;
                --vsi;
                memcpy(&val_stack[vsi - 1], result, sizeof(Value));
                top->name = NULL;
                --osi;
            }

            check_stack(osi);
            memcpy(&op_stack[osi], op, sizeof(Operator));
            ++osi;
            expect_operand = true;
        }
        else
        {
            Streader_set_error(sr, "Unrecognised token");
            return false;
        }

        prev_pos = sr->pos;
    }

    if (Streader_is_error_set(sr))
        return false;

    rassert(string_eq(token, "") || string_eq(token, ")") ||
            (func_arg && string_eq(token, ",")));
    if (vsi <= orig_vsi)
    {
        rassert(vsi == orig_vsi);
        Streader_set_error(sr, "Empty expression");
        return false;
    }

    if ((depth == 0) != string_eq(token, ""))
    {
        Streader_set_error(
                sr,
                "Unmatched %s parenthesis",
                (depth == 0) ? "right" : "left");
        return false;
    }

    while (osi > orig_osi)
    {
        Operator* top = &op_stack[osi - 1];
        rassert(top->name != NULL);
        rassert(top->func != NULL);

        if (vsi < 2)
        {
            Streader_set_error(sr, "Not enough operands");
            return false;
        }

        Value* result = VALUE_AUTO;
        if (!top->func(&val_stack[vsi - 2], &val_stack[vsi - 1],
                       result, sr))
        {
            rassert(Streader_is_error_set(sr));
            return false;
        }

        //Operator_print(top);
        rassert(result->type != VALUE_TYPE_NONE);
        val_stack[vsi - 2].type = val_stack[vsi - 1].type = VALUE_TYPE_NONE;
        --vsi;
        memcpy(&val_stack[vsi - 1], result, sizeof(Value));
        top->name = NULL;
        --osi;
    }

    rassert(vsi > orig_vsi);
    memcpy(res, &val_stack[vsi - 1], sizeof(Value));
    rassert(res->type != VALUE_TYPE_NONE);

    if (func_arg)
        sr->pos = prev_pos;

    return true;
}

#undef check_stack


static bool token_is_func(const char* token, Func* res)
{
    rassert(token != NULL);
    rassert(res != NULL);

    for (int i = 0; funcs[i].name != NULL; ++i)
    {
        if (string_eq(funcs[i].name, token))
        {
            *res = funcs[i].func;
            return true;
        }
    }

    return false;
}


static bool handle_unary(Value* val, bool found_not, bool found_minus, Streader* sr)
{
    rassert(val != NULL);
    rassert(!(found_not && found_minus));
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!found_not && !found_minus)
        return true;

    if (found_not)
    {
        if (val->type != VALUE_TYPE_BOOL)
        {
            Streader_set_error(sr, "Non-boolean operand for boolean not");
            return false;
        }

        val->value.bool_type = !val->value.bool_type;
        return true;
    }

    rassert(found_minus);
    switch (val->type)
    {
        case VALUE_TYPE_INT:
        {
            val->value.int_type = -val->value.int_type;
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            val->value.float_type = -val->value.float_type;
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            const Tstamp* zero_ts = Tstamp_init(TSTAMP_AUTO);
            Tstamp_sub(&val->value.Tstamp_type, zero_ts, &val->value.Tstamp_type);
        }
        break;

        default:
        {
            Streader_set_error(sr, "Non-number operand for unary minus");
            return false;
        }
    }

    return true;
}


static bool Value_from_token(
        Value* val, char* token, Env_state* estate, const Value* meta)
{
    rassert(val != NULL);
    rassert(token != NULL);
    rassert(meta != NULL);

    if (isdigit(token[0]) || token[0] == '.')
    {
        const int64_t token_length = (int64_t)strlen(token);
        if (strchr(token, '.') != NULL)
        {
            Streader* sr = Streader_init(STREADER_AUTO, token, token_length);
            double num = NAN;
            if (!Streader_read_float(sr, &num))
                return false;

            val->type = VALUE_TYPE_FLOAT;
            val->value.float_type = num;
            return true;
        }

        Streader* sr = Streader_init(STREADER_AUTO, token, token_length);
        int64_t num = 0;
        if (!Streader_read_int(sr, &num))
            return false;

        val->type = VALUE_TYPE_INT;
        val->value.int_type = num;
        return true;
    }
    else if (token[0] == '\'' || string_has_prefix(token, "\\\""))
    {
        const char* end_str = "'";
        ++token;
        if (token[0] == '"')
        {
            end_str = "\\\"";
            ++token;
        }

        int i = 0;
        while (!string_has_prefix(&token[i], end_str))
        {
            if (i >= KQT_VAR_NAME_MAX)
            {
                return false;
            }
            val->value.string_type[i] = token[i];
            ++i;
        }

        val->type = VALUE_TYPE_STRING;
        val->value.string_type[i] = '\0';

        return true;
    }
    else if (string_eq(token, "true") || string_eq(token, "false"))
    {
        val->type = VALUE_TYPE_BOOL;
        val->value.bool_type = string_eq(token, "true");
        return true;
    }
    else if (string_eq(token, "$"))
    {
        Value_copy(val, meta);
        return true;
    }
    else if (strchr(KQT_VAR_INIT_CHARS, token[0]) != NULL)
    {
        const Env_var* ev = (estate != NULL) ? Env_state_get_var(estate, token) : NULL;
        if (ev == NULL)
            return false;

        rassert(Env_var_get_type(ev) == VALUE_TYPE_BOOL ||
                Env_var_get_type(ev) == VALUE_TYPE_INT ||
                Env_var_get_type(ev) == VALUE_TYPE_FLOAT ||
                Env_var_get_type(ev) == VALUE_TYPE_TSTAMP);

        Value_copy(val, Env_var_get_value(ev));

        return true;
    }

    return false;
}


#if 0
static void Value_print(Value* val)
{
    rassert(val != NULL);

    switch (val->type)
    {
        case VALUE_TYPE_BOOL:
        {
            fprintf(stderr, "%s", val->value.bool_type ? "true" : "false");
        } break;
        case VALUE_TYPE_INT:
        {
            fprintf(stderr, "%lld", (long long)val->value.int_type);
        } break;
        case VALUE_TYPE_FLOAT:
        {
            fprintf(stderr, "%f", val->value.float_type);
        } break;
        default:
            rassert(false);
    }

    fprintf(stderr, " ");

    return;
}
#endif


static bool Operator_from_token(Operator* op, char* token)
{
    rassert(op != NULL);
    rassert(token != NULL);

    for (int i = 0; operators[i].name != NULL; ++i)
    {
        if (string_eq(token, operators[i].name))
        {
            memcpy(op, &operators[i], sizeof(Operator));
            return true;
        }
    }

    return false;
}


#if 0
static void Operator_print(Operator* op)
{
    rassert(op != NULL);
    rassert(op->name != NULL);
    fprintf(stderr, "%s ", op->name);
    return;
}
#endif


static bool get_token(Streader* sr, char* result)
{
    rassert(sr != NULL);
    rassert(result != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);
    const char* str = &sr->str[sr->pos]; // FIXME: need to handle this properly
    if (isdigit(*str) || *str == '.')
    {
        return get_num_token(sr, result);
    }
    else if (str[0] == '\'' || string_has_prefix(str, "\\\""))
    {
        return get_str_token(sr, result);
    }
    else if (Streader_try_match_char(sr, '$'))
    {
        strcpy(result, "$");
        return true;
    }
    else if (Streader_try_match_char(sr, ','))
    {
        strcpy(result, ",");
        return true;
    }
    else if (strchr(KQT_VAR_INIT_CHARS, *str) != NULL)
    {
        return get_var_token(sr, result);
    }
    else if (Streader_try_match_char(sr, '('))
    {
        strcpy(result, "(");
        return true;
    }
    else if (Streader_try_match_char(sr, ')'))
    {
        strcpy(result, ")");
        return true;
    }

    return get_op_token(sr, result);
}


static bool get_num_token(Streader* sr, char* result)
{
    rassert(sr != NULL);
    rassert(result != NULL);

    const int64_t start_pos = sr->pos;

    if (!Streader_read_float(sr, NULL))
        return false;

    const int64_t end_pos = sr->pos;
    rassert(end_pos >= start_pos);

    const int64_t len = end_pos - start_pos;
    if (len > KQT_VAR_NAME_MAX)
    {
        Streader_set_error(sr, "Exceeded maximum token length");
        return false;
    }

    strncpy(result, &sr->str[start_pos], (size_t)len);
    result[len] = '\0';

    return true;
}


static bool get_str_token(Streader* sr, char* result)
{
    rassert(sr != NULL);
    rassert(result != NULL);

    // FIXME: ugly haxoring with Streader internals
    const char* str = &sr->str[sr->pos];

    const char* end_str = "'";
    if (string_has_prefix(str, "\\\""))
        end_str = "\\\"";

    result[0] = *str++;
    int i = 1;
    while (!string_has_prefix(str, end_str))
    {
        if (i > KQT_VAR_NAME_MAX + 1) // + 1 includes compensation for \"
        {
            Streader_set_error(sr, "Exceeded maximum token length");
            return false;
        }

        result[i] = *str++;
        ++i;
    }

    strcpy(&result[i], end_str);
    str += strlen(end_str);

    sr->pos += (int)(str - &sr->str[sr->pos]);

    return true;
}


static bool get_var_token(Streader* sr, char* result)
{
    rassert(sr != NULL);
    rassert(result != NULL);

    // FIXME: ugly haxoring with Streader internals
    const char* str = &sr->str[sr->pos];

    const size_t len = strspn(str, KQT_VAR_CHARS);
    if (len > KQT_VAR_NAME_MAX)
    {
        Streader_set_error(sr, "Exceeded maximum token length");
        return false;
    }

    strncpy(result, str, len);
    result[len] = '\0';

    sr->pos += (int)len;

    return true;
}


static bool get_op_token(Streader* sr, char* result)
{
    rassert(sr != NULL);
    rassert(result != NULL);

    static const char op_chars[] = "!=<>+-*/%^|&";

    // FIXME: ugly haxoring with Streader internals
    const char* str = &sr->str[sr->pos];

    const size_t len = strspn(str, op_chars);
    if (len > KQT_VAR_NAME_MAX)
    {
        Streader_set_error(sr, "Exceeded maximum token length");
        return false;
    }

    strncpy(result, str, len);
    result[len] = '\0';

    sr->pos += (int)len;

    return true;
}


static bool promote_arithmetic_types(
        Value* pr_op1, Value* pr_op2, const Value* op1, const Value* op2, Streader* sr)
{
    rassert(pr_op1 != NULL);
    rassert(pr_op2 != NULL);
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    // Verify that both types are arithmetic
    if ((op1->type < VALUE_TYPE_INT) || (op1->type > VALUE_TYPE_TSTAMP) ||
            (op2->type < VALUE_TYPE_INT) || (op2->type > VALUE_TYPE_TSTAMP))
    {
        Streader_set_error(sr, "Non-arithmetic type used in arithmetic expression");
        return false;
    }

    // Types match
    if (op1->type == op2->type)
    {
        Value_copy(pr_op1, op1);
        Value_copy(pr_op2, op2);
        return true;
    }

    // Convert the lower type in the type hierarchy
    static const int type_prio[VALUE_TYPE_COUNT] =
    {
        [VALUE_TYPE_INT] = 1,
        [VALUE_TYPE_TSTAMP] = 2,
        [VALUE_TYPE_FLOAT] = 3,
    };

    bool success = false;

    if (type_prio[op1->type] < type_prio[op2->type])
    {
        success = Value_convert(pr_op1, op1, op2->type);
        Value_copy(pr_op2, op2);
    }
    else
    {
        Value_copy(pr_op1, op1);
        success = Value_convert(pr_op2, op2, op1->type);
    }

    if (!success)
    {
        Streader_set_error(sr, "Could not promote operand type");
        return false;
    }

    rassert(pr_op1->type == pr_op2->type);

    return true;
}


static bool op_eq(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);
    rassert(op1->type > VALUE_TYPE_NONE);
    rassert(op2->type > VALUE_TYPE_NONE);

    if (Streader_is_error_set(sr))
        return false;

    // Eliminate testing of symmetric cases
    if (op1->type > op2->type)
    {
        const Value* tmp = op1;
        op1 = op2;
        op2 = tmp;
    }

    res->type = VALUE_TYPE_BOOL;

    // Check non-arithmetic types first
    if (op1->type == VALUE_TYPE_BOOL)
    {
        if (op1->type != op2->type)
        {
            Streader_set_error(sr, "Comparison between boolean and non-boolean");
            return false;
        }

        res->value.bool_type = (op1->value.bool_type == op2->value.bool_type);
        return true;
    }
    else if (op2->type == VALUE_TYPE_STRING)
    {
        if (op1->type != op2->type)
        {
            Streader_set_error(sr, "Comparison between string and non-string");
            return false;
        }

        res->value.bool_type = string_eq(op1->value.string_type, op2->value.string_type);
        return true;
    }

    // Compare arithmetic types
    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            res->value.bool_type = (pr_op1->value.int_type == pr_op2->value.int_type);
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            res->value.bool_type =
                (pr_op1->value.float_type == pr_op2->value.float_type);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            res->value.bool_type =
                (Tstamp_cmp(&pr_op1->value.Tstamp_type, &pr_op2->value.Tstamp_type) == 0);
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool op_neq(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (op_eq(op1, op2, res, sr))
    {
        rassert(res->type == VALUE_TYPE_BOOL);
        res->value.bool_type = !res->value.bool_type;
        return true;
    }

    return false;
}


static bool op_leq(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (!op_lt(op1, op2, res, sr))
        return false;

    if (res->value.bool_type)
        return true;

    return op_eq(op1, op2, res, sr);
}


static bool op_geq(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    return op_leq(op2, op1, res, sr);
}


static bool op_or(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (op1->type != VALUE_TYPE_BOOL || op2->type != VALUE_TYPE_BOOL)
    {
        Streader_set_error(sr, "Boolean OR with non-booleans");
        return false;
    }

    res->type = VALUE_TYPE_BOOL;
    res->value.bool_type = op1->value.bool_type || op2->value.bool_type;

    return true;
}


static bool op_and(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (op1->type != VALUE_TYPE_BOOL || op2->type != VALUE_TYPE_BOOL)
    {
        Streader_set_error(sr, "Boolean AND with non-booleans");
        return false;
    }

    res->type = VALUE_TYPE_BOOL;
    res->value.bool_type = op1->value.bool_type && op2->value.bool_type;

    return true;
}


static bool op_lt(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Streader_set_error(sr, "Ordinal comparison between non-arithmetic types");
        return false;
    }

    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    res->type = VALUE_TYPE_BOOL;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            res->value.bool_type = (pr_op1->value.int_type < pr_op2->value.int_type);
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            res->value.bool_type = (pr_op1->value.float_type < pr_op2->value.float_type);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            res->value.bool_type =
                (Tstamp_cmp(&pr_op1->value.Tstamp_type, &pr_op2->value.Tstamp_type) < 0);
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool op_gt(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    return op_lt(op2, op1, res, sr);
}


static bool op_add(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    res->type = pr_op1->type;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            res->value.int_type = pr_op1->value.int_type + pr_op2->value.int_type;
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            res->value.float_type = pr_op1->value.float_type + pr_op2->value.float_type;
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            Tstamp_add(
                    &res->value.Tstamp_type,
                    &pr_op1->value.Tstamp_type,
                    &pr_op2->value.Tstamp_type);
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool op_sub(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Streader_set_error(sr, "Subtraction with non-numbers");
        return false;
    }

    // Negate the second operand
    Value* neg_op2 = Value_copy(VALUE_AUTO, op2);

    if (neg_op2->type == VALUE_TYPE_INT)
    {
        neg_op2->value.int_type = -neg_op2->value.int_type;
    }
    else if (neg_op2->type == VALUE_TYPE_FLOAT)
    {
        neg_op2->value.float_type = -neg_op2->value.float_type;
    }
    else if (neg_op2->type == VALUE_TYPE_TSTAMP)
    {
        const Tstamp* zero_ts = Tstamp_init(TSTAMP_AUTO);
        Tstamp_sub(&neg_op2->value.Tstamp_type, zero_ts, &neg_op2->value.Tstamp_type);
    }
    else
    {
        rassert(false);
    }

    return op_add(op1, neg_op2, res, sr);
}


static bool op_mul(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    res->type = pr_op1->type;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            res->value.int_type = pr_op1->value.int_type * pr_op2->value.int_type;
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            res->value.float_type = pr_op1->value.float_type * pr_op2->value.float_type;
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            Value_convert(pr_op1, pr_op1, VALUE_TYPE_FLOAT);
            Value_convert(pr_op2, pr_op2, VALUE_TYPE_FLOAT);

            res->type = VALUE_TYPE_FLOAT;
            res->value.float_type = pr_op1->value.float_type * pr_op2->value.float_type;
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool op_div(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    res->type = pr_op1->type;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            if (pr_op2->value.int_type == 0)
            {
                Streader_set_error(sr, "Division by zero");
                return false;
            }

            if (pr_op1->value.int_type % pr_op2->value.int_type == 0)
            {
                res->type = VALUE_TYPE_INT;
                res->value.int_type = pr_op1->value.int_type / pr_op2->value.int_type;
            }
            else
            {
                res->type = VALUE_TYPE_FLOAT;
                res->value.float_type =
                    (double)pr_op1->value.float_type / (double)pr_op2->value.float_type;
            }
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            if (pr_op2->value.float_type == 0)
            {
                Streader_set_error(sr, "Division by zero");
                return false;
            }

            res->value.float_type = pr_op1->value.float_type / pr_op2->value.float_type;
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            Value_convert(pr_op1, pr_op1, VALUE_TYPE_FLOAT);
            Value_convert(pr_op2, pr_op2, VALUE_TYPE_FLOAT);

            if (pr_op2->value.float_type == 0)
            {
                Streader_set_error(sr, "Division by zero");
                return false;
            }

            res->type = VALUE_TYPE_FLOAT;
            res->value.float_type = pr_op1->value.float_type / pr_op2->value.float_type;
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool op_mod(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    res->type = pr_op1->type;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            if (pr_op2->value.int_type == 0)
            {
                Streader_set_error(sr, "Modulo by zero");
                return false;
            }

            res->value.int_type = pr_op1->value.int_type % pr_op2->value.int_type;
            if (res->value.int_type < 0)
                res->value.int_type += pr_op2->value.int_type;
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            if (pr_op2->value.float_type == 0)
            {
                Streader_set_error(sr, "Modulo by zero");
                return false;
            }

            const double divisor = pr_op2->value.float_type;
            res->value.float_type = fmod(pr_op1->value.float_type, divisor);
            if (res->value.float_type < 0)
                res->value.float_type += divisor;
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            Value_convert(pr_op1, pr_op1, VALUE_TYPE_FLOAT);
            Value_convert(pr_op2, pr_op2, VALUE_TYPE_FLOAT);

            if (pr_op2->value.float_type == 0)
            {
                Streader_set_error(sr, "Modulo by zero");
                return false;
            }

            res->type = VALUE_TYPE_FLOAT;

            const double divisor = pr_op2->value.float_type;
            res->value.float_type = fmod(pr_op1->value.float_type, divisor);
            if (res->value.float_type < 0)
                res->value.float_type += divisor;
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool float_pow(const Value* pr_op1, const Value* pr_op2, Value* res, Streader* sr)
{
    rassert(pr_op1 != NULL);
    rassert(pr_op1->type == VALUE_TYPE_FLOAT);
    rassert(pr_op2 != NULL);
    rassert(pr_op2->type == VALUE_TYPE_FLOAT);
    rassert(res != NULL);
    rassert(sr != NULL);
    rassert(!Streader_is_error_set(sr));

    if ((pr_op1->value.float_type == 0) && (pr_op2->value.float_type == 0))
    {
        Streader_set_error(sr, "0 ^ 0 is undefined");
        return false;
    }

    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = pow(pr_op1->value.float_type, pr_op2->value.float_type);

    return true;
}


static bool op_pow(const Value* op1, const Value* op2, Value* res, Streader* sr)
{
    rassert(op1 != NULL);
    rassert(op2 != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Value* pr_op1 = VALUE_AUTO;
    Value* pr_op2 = VALUE_AUTO;

    if (!promote_arithmetic_types(pr_op1, pr_op2, op1, op2, sr))
        return false;

    res->type = pr_op1->type;

    switch (pr_op1->type)
    {
        case VALUE_TYPE_INT:
        {
            if (pr_op2->value.int_type >= 0)
            {
                if ((pr_op1->value.int_type == 0) && (pr_op2->value.int_type == 0))
                {
                    Streader_set_error(sr, "0 ^ 0 is undefined");
                    return false;
                }

                res->type = VALUE_TYPE_INT;
                res->value.int_type = ipowi(
                        pr_op1->value.int_type, pr_op2->value.int_type);
            }
            else
            {
                Value_convert(pr_op1, pr_op1, VALUE_TYPE_FLOAT);
                Value_convert(pr_op2, pr_op2, VALUE_TYPE_FLOAT);
                return float_pow(pr_op1, pr_op2, res, sr);
            }
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            return float_pow(pr_op1, pr_op2, res, sr);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            Value_convert(pr_op1, pr_op1, VALUE_TYPE_FLOAT);
            Value_convert(pr_op2, pr_op2, VALUE_TYPE_FLOAT);
            return float_pow(pr_op1, pr_op2, res, sr);
        }
        break;

        default:
            rassert(false);
    }

    return true;
}


static bool func_ts(const Value* args, Value* res, Random* rand, Streader* sr)
{
    rassert(args != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);
    ignore(rand);

    if (Streader_is_error_set(sr))
        return false;

    res->type = VALUE_TYPE_TSTAMP;
    Tstamp_init(&res->value.Tstamp_type);
    if (args[0].type == VALUE_TYPE_NONE)
    {
        return true;
    }
    else if (args[0].type == VALUE_TYPE_TSTAMP)
    {
        Value_copy(res, &args[0]);
        return true;
    }
    else if (args[0].type == VALUE_TYPE_INT)
    {
        Tstamp_set(&res->value.Tstamp_type, args[0].value.int_type, 0);
    }
    else if (args[0].type == VALUE_TYPE_FLOAT)
    {
        double beats = floor(args[0].value.float_type);
        Tstamp_set(
                &res->value.Tstamp_type,
                (int64_t)beats,
                (int32_t)((args[0].value.float_type - beats) * KQT_TSTAMP_BEAT));
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Streader_set_error(sr, "Invalid beat type");
        return false;
    }

    if (args[1].type == VALUE_TYPE_NONE)
    {
        return true;
    }
    else if (args[1].type == VALUE_TYPE_INT)
    {
        if (args[1].value.int_type < 0 ||
                args[1].value.int_type >= KQT_TSTAMP_BEAT)
        {
            res->type = VALUE_TYPE_NONE;
            Streader_set_error(sr, "Invalid beat value");
            return false;
        }
        Tstamp_add(
                &res->value.Tstamp_type, &res->value.Tstamp_type,
                Tstamp_set(TSTAMP_AUTO, 0, (int32_t)args[1].value.int_type));
    }
    else if (args[1].type == VALUE_TYPE_FLOAT)
    {
        if (args[1].value.float_type < 0 ||
                args[1].value.float_type >= KQT_TSTAMP_BEAT)
        {
            res->type = VALUE_TYPE_NONE;
            Streader_set_error(sr, "Invalid beat value");
            return false;
        }
        Tstamp_add(
                &res->value.Tstamp_type, &res->value.Tstamp_type,
                Tstamp_set(TSTAMP_AUTO, 0, (int32_t)args[1].value.float_type));
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Streader_set_error(sr, "Invalid remainder type");
        return false;
    }

    return true;
}


static bool func_rand(const Value* args, Value* res, Random* rand, Streader* sr)
{
    rassert(args != NULL);
    rassert(res != NULL);
    rassert(rand != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = Random_get_float_lb(rand);
    if (args[0].type == VALUE_TYPE_NONE)
    {
    }
    else if (args[0].type == VALUE_TYPE_FLOAT)
    {
        res->value.float_type = res->value.float_type * args[0].value.float_type;
    }
    else if (args[0].type == VALUE_TYPE_INT)
    {
        res->value.float_type = res->value.float_type * (double)args[0].value.int_type;
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Streader_set_error(sr, "Invalid argument");
        return false;
    }

    return true;
}


static bool func_pat(const Value* args, Value* res, Random* rand, Streader* sr)
{
    rassert(args != NULL);
    rassert(res != NULL);
    rassert(sr != NULL);
    ignore(rand);

    if (Streader_is_error_set(sr))
        return false;

    res->type = VALUE_TYPE_PAT_INST_REF;
    res->value.Pat_inst_ref_type = *PAT_INST_REF_AUTO;

    // Read pattern number
    if (args[0].type == VALUE_TYPE_NONE)
    {
        return true;
    }
    else if (args[0].type == VALUE_TYPE_PAT_INST_REF)
    {
        Value_copy(res, &args[0]);
        return true;
    }
    else if (args[0].type == VALUE_TYPE_INT)
    {
        if (args[0].value.int_type < 0 ||
                args[0].value.int_type >= KQT_PATTERNS_MAX)
        {
            res->type = VALUE_TYPE_NONE;
            Streader_set_error(sr, "Invalid pattern number");
            return false;
        }
        res->value.Pat_inst_ref_type.pat = (int16_t)args[0].value.int_type;
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Streader_set_error(sr, "Invalid pattern value type");
        return false;
    }

    // Read instance number
    if (args[1].type == VALUE_TYPE_NONE)
    {
        return true;
    }
    else if (args[1].type == VALUE_TYPE_INT)
    {
        if (args[1].value.int_type < 0 ||
                args[1].value.int_type >= KQT_PAT_INSTANCES_MAX)
        {
            res->type = VALUE_TYPE_NONE;
            Streader_set_error(sr, "Invalid pattern instance value");
            return false;
        }
        res->value.Pat_inst_ref_type.inst = (int16_t)args[1].value.int_type;
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Streader_set_error(sr, "Invalid pattern instance value type");
        return false;
    }

    return true;
}


