

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

#include <expr.h>
#include <File_base.h>
#include <math_common.h>
#include <string_common.h>
#include <xassert.h>


#define STACK_SIZE 32


typedef bool (*Op_func)(Value* op1, Value* op2, Value* res, Read_state* state);


typedef struct Operator
{
    char* name;
    int preced;
    Op_func func;
} Operator;


static char* evaluate_expr_(char* str, Environment* env, Read_state* state,
                            Value* val_stack, int vsi,
                            Operator* op_stack, int osi,
                            Value* meta, Value* res, int depth,
                            bool func_arg);


static bool handle_unary(Value* val, bool found_not, bool found_minus,
                         Read_state* state);


static char* get_token(char* str, char* result, Read_state* state);
static char* get_str_token(char* str, char* result, Read_state* state);
static char* get_num_token(char* str, char* result, Read_state* state);
static char* get_var_token(char* str, char* result, Read_state* state);
static char* get_op_token(char* str, char* result, Read_state* state);


static bool Value_from_token(Value* val, char* token,
                             Environment* env, Value* meta);

//static void Value_print(Value* val);


#define OPERATOR_AUTO (&(Operator){ .name = NULL, .func = NULL })


static bool Operator_from_token(Operator* op, char* token);

//static void Operator_print(Operator* op);


static bool op_neq(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_leq(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_geq(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_or(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_and(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_eq(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_lt(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_gt(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_add(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_sub(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_mul(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_div(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_mod(Value* op1, Value* op2, Value* res, Read_state* state);
static bool op_pow(Value* op1, Value* op2, Value* res, Read_state* state);


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


typedef bool (*Func)(Value* args, Value* res, Read_state* state);


#define FUNC_ARGS_MAX 4


static bool token_is_func(char* token, Func* res);


typedef struct Func_desc
{
    char* name;
    Func func;
} Func_desc;


static bool func_ts(Value* args, Value* res, Read_state* state);


static Func_desc funcs[] =
{
    { .name = "ts", .func = func_ts },
    { .name = NULL, .func = NULL }
};


char* evaluate_expr(char* str,
                    Environment* env,
                    Read_state* state,
                    Value* meta,
                    Value* res)
{
    assert(str != NULL);
    assert(env != NULL);
    assert(state != NULL);
    assert(res != NULL);
    if (state->error)
    {
        return str;
    }
    Value val_stack[STACK_SIZE] = { { .type = VALUE_TYPE_NONE } };
    Operator op_stack[STACK_SIZE] = { { .name = NULL } };
    if (meta == NULL)
    {
        meta = VALUE_AUTO;
    }
    return evaluate_expr_(str, env, state, val_stack, 0, op_stack, 0,
                          meta, res, 0, false);
}


#define check_stack(si) if (true)                           \
    {                                                       \
        if ((si) >= STACK_SIZE)                             \
        {                                                   \
            assert((si) == STACK_SIZE);                     \
            Read_state_set_error(state, "Stack overflow");  \
            return false;                                   \
        }                                                   \
    } else (void)0

static char* evaluate_expr_(char* str, Environment* env, Read_state* state,
                            Value* val_stack, int vsi,
                            Operator* op_stack, int osi,
                            Value* meta, Value* res, int depth,
                            bool func_arg)
{
    assert(str != NULL);
    assert(env != NULL);
    assert(state != NULL);
    assert(val_stack != NULL);
    assert(vsi >= 0);
    assert(vsi <= STACK_SIZE);
    assert(op_stack != NULL);
    assert(osi >= 0);
    assert(osi <= STACK_SIZE);
    assert(meta != NULL);
    assert(res != NULL);
    assert(depth >= 0);
    if (state->error)
    {
        return str;
    }
    if (depth >= STACK_SIZE)
    {
        Read_state_set_error(state, "Maximum recursion depth exceeded");
        return str;
    }
    int orig_vsi = vsi;
    int orig_osi = osi;
    char token[ENV_VAR_NAME_MAX + 4] = ""; // + 4 for delimiting \"s
    bool expect_operand = true;
    bool found_not = false;
    bool found_minus = false;
    char* prev_pos = str;
    str = get_token(str, token, state);
    while (!state->error && !string_eq(token, "") &&
            !string_eq(token, ")") && (!func_arg || !string_eq(token, ",")))
    {
        Value* operand = VALUE_AUTO;
        Operator* op = OPERATOR_AUTO;
        Func func = NULL;
        if (string_eq(token, "("))
        {
            if (!expect_operand)
            {
                Read_state_set_error(state, "Unexpected operand");
                return str;
            }
            check_stack(vsi);
            str = evaluate_expr_(str, env, state, val_stack, vsi,
                                 op_stack, osi, meta, operand, depth + 1,
                                 false);
            if (state->error)
            {
                return str;
            }
            assert(operand->type != VALUE_TYPE_NONE);
            if (!handle_unary(operand, found_not, found_minus, state))
            {
                return str;
            }
            found_not = found_minus = false;
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
        }
        else if (token_is_func(token, &func))
        {
            if (!expect_operand)
            {
                Read_state_set_error(state, "Unexpected function");
                return str;
            }
            check_stack(vsi);
            assert(func != NULL);
            Value func_args[FUNC_ARGS_MAX] = { { .type = VALUE_TYPE_NONE } };
            str = read_const_char(str, '(', state);
            if (state->error)
            {
                return str;
            }
            int i = 0;
            str = read_const_char(str, ')', state);
            if (state->error)
            {
                Read_state_clear_error(state);
                for (i = 0; i < FUNC_ARGS_MAX; ++i)
                {
                    str = evaluate_expr_(str, env, state, val_stack, vsi,
                                         op_stack, osi, meta, &func_args[i],
                                         depth + 1, true);
                    if (state->error)
                    {
                        return str;
                    }
                    assert(func_args[i].type != VALUE_TYPE_NONE);
                    str = read_const_char(str, ')', state);
                    if (!state->error)
                    {
                        break;
                    }
                    Read_state_clear_error(state);
                    str = read_const_char(str, ',', state);
                    if (state->error)
                    {
                        return str;
                    }
                }
            }
            if (i < FUNC_ARGS_MAX)
            {
                func_args[i].type = VALUE_TYPE_NONE;
            }
            if (!func(func_args, operand, state))
            {
                assert(state->error);
                return str;
            }
            assert(operand->type != VALUE_TYPE_NONE);
            found_not = found_minus = false;
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
        }
        else if (Value_from_token(operand, token, env, meta))
        {
            if (!expect_operand)
            {
                Read_state_set_error(state, "Unexpected operand");
                return str;
            }
            assert(operand->type != VALUE_TYPE_NONE);
            if (!handle_unary(operand, found_not, found_minus, state))
            {
                return str;
            }
            found_not = found_minus = false;
            check_stack(vsi);
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
            //Value_print(operand);
        }
        else if (Operator_from_token(op, token))
        {
            assert(op->name != NULL);
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
                    Read_state_set_error(state, "Unexpected binary operator");
                    return str;
                }
                prev_pos = str;
                str = get_token(str, token, state);
                continue;
            }
            if (string_eq(op->name, "!"))
            {
                Read_state_set_error(state, "Unexpected boolean not");
                return str;
            }
            while (osi > orig_osi && op->preced <= op_stack[osi - 1].preced)
            {
                Operator* top = &op_stack[osi - 1];
                assert(top->name != NULL);
                assert(top->func != NULL);
                if (vsi < 2)
                {
                    Read_state_set_error(state, "Not enough operands");
                    return str;
                }
                Value* result = VALUE_AUTO;
                if (!top->func(&val_stack[vsi - 2], &val_stack[vsi - 1],
                               result, state))
                {
                    assert(state->error);
                    return str;
                }
                //Operator_print(top);
                assert(result->type != VALUE_TYPE_NONE);
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
            Read_state_set_error(state, "Unrecognised token");
            return str;
        }
        prev_pos = str;
        str = get_token(str, token, state);
    }
    if (state->error)
    {
        return str;
    }
    assert(string_eq(token, "") || string_eq(token, ")"));
    if (vsi <= orig_vsi)
    {
        assert(vsi == orig_vsi);
        Read_state_set_error(state, "Empty expression");
        return str;
    }
    if ((depth == 0) != string_eq(token, ""))
    {
        Read_state_set_error(state, "Unmatched %s parenthesis",
                             depth == 0 ? "right" : "left");
        return str;
    }
    while (osi > orig_osi)
    {
        Operator* top = &op_stack[osi - 1];
        assert(top->name != NULL);
        assert(top->func != NULL);
        if (vsi < 2)
        {
            Read_state_set_error(state, "Not enough operands");
            return str;
        }
        Value* result = VALUE_AUTO;
        if (!top->func(&val_stack[vsi - 2], &val_stack[vsi - 1],
                       result, state))
        {
            assert(state->error);
            return str;
        }
        //Operator_print(top);
        assert(result->type != VALUE_TYPE_NONE);
        val_stack[vsi - 2].type = val_stack[vsi - 1].type = VALUE_TYPE_NONE;
        --vsi;
        memcpy(&val_stack[vsi - 1], result, sizeof(Value));
        top->name = NULL;
        --osi;
    }
    assert(vsi > orig_vsi);
    memcpy(res, &val_stack[vsi - 1], sizeof(Value));
    assert(res->type != VALUE_TYPE_NONE);
    if (func_arg)
    {
        str = prev_pos;
    }
    return str;
}

#undef check_stack


static bool token_is_func(char* token, Func* res)
{
    assert(token != NULL);
    assert(res != NULL);
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


static bool handle_unary(Value* val, bool found_not, bool found_minus,
                         Read_state* state)
{
    assert(val != NULL);
    assert(!(found_not && found_minus));
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (!found_not && !found_minus)
    {
        return true;
    }
    if (found_not)
    {
        if (val->type != VALUE_TYPE_BOOL)
        {
            Read_state_set_error(state, "Non-boolean operand for"
                                        " boolean not");
            return false;
        }
        val->value.bool_type = !val->value.bool_type;
        return true;
    }
    assert(found_minus);
    if (val->type == VALUE_TYPE_INT)
    {
        val->value.int_type = -val->value.int_type;
    }
    else if (val->type == VALUE_TYPE_FLOAT)
    {
        val->value.float_type = -val->value.float_type;
    }
    else
    {
        Read_state_set_error(state, "Non-number operand for unary minus");
        return false;
    }
    return true;
}


static bool Value_from_token(Value* val, char* token,
                             Environment* env, Value* meta)
{
    assert(val != NULL);
    assert(token != NULL);
    assert(env != NULL);
    assert(meta != NULL);
    if (isdigit(token[0]) || token[0] == '.')
    {
        char* endptr = NULL;
        if (strchr(token, '.') != NULL)
        {
            errno = 0;
            double num = strtod(token, &endptr);
            if (errno || *endptr != '\0')
            {
                return false;
            }
            val->type = VALUE_TYPE_FLOAT;
            val->value.float_type = num;
            return true;
        }
        errno = 0;
        long long num = strtoll(token, &endptr, 10);
        if (errno || *endptr != '\0')
        {
            return false;
        }
        val->type = VALUE_TYPE_INT;
        val->value.int_type = num;
        return true;
    }
    else if (token[0] == '\'' || string_has_prefix(token, "\\\""))
    {
        char* end_str = "'";
        ++token;
        if (string_has_prefix(token, "\\\""))
        {
            end_str = "\\\"";
            ++token;
        }
        int i = 0;
        while (!string_has_prefix(&token[i], end_str))
        {
            if (i >= ENV_VAR_NAME_MAX - 1)
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
    else if (strchr(ENV_VAR_INIT_CHARS, token[0]) != NULL)
    {
        Env_var* ev = Environment_get(env, token);
        if (ev == NULL)
        {
            return false;
        }
        switch (Env_var_get_type(ev))
        {
            case ENV_VAR_BOOL:
            {
                bool* ev_val = Env_var_get_value(ev);
                assert(ev_val != NULL);
                val->type = VALUE_TYPE_BOOL;
                val->value.bool_type = *ev_val;
            } break;
            case ENV_VAR_INT:
            {
                int64_t* ev_val = Env_var_get_value(ev);
                assert(ev_val != NULL);
                val->type = VALUE_TYPE_INT;
                val->value.int_type = *ev_val;
            } break;
            case ENV_VAR_FLOAT:
            {
                double* ev_val = Env_var_get_value(ev);
                assert(ev_val != NULL);
                val->type = VALUE_TYPE_FLOAT;
                val->value.float_type = *ev_val;
            } break;
            default:
                assert(false);
        }
        return true;
    }
    return false;
}


#if 0
static void Value_print(Value* val)
{
    assert(val != NULL);
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
            assert(false);
    }
    fprintf(stderr, " ");
    return;
}
#endif


static bool Operator_from_token(Operator* op, char* token)
{
    assert(op != NULL);
    assert(token != NULL);
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
    assert(op != NULL);
    assert(op->name != NULL);
    fprintf(stderr, "%s ", op->name);
    return;
}
#endif


static char* get_token(char* str, char* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = skip_whitespace(str, state);
    if (isdigit(*str) || *str == '.')
    {
        return get_num_token(str, result, state);
    }
    else if (str[0] == '\'' || string_has_prefix(str, "\\\""))
    {
        return get_str_token(str, result, state);
    }
    else if (strchr("$,", str[0]) != NULL)
    {
        result[0] = str[0];
        result[1] = '\0';
        return str + 1;
    }
    else if (strchr(ENV_VAR_INIT_CHARS, *str) != NULL)
    {
        return get_var_token(str, result, state);
    }
    else if (strchr("()", *str) != NULL)
    {
        result[0] = *str;
        result[1] = '\0';
        ++str;
        return str;
    }
    return get_op_token(str, result, state);
}


static char* get_num_token(char* str, char* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    int len = 0;
    while (isdigit(*str) && len < ENV_VAR_NAME_MAX - 1)
    {
        result[len++] = *str++;
    }
    if (*str == '.' && len < ENV_VAR_NAME_MAX - 1)
    {
        result[len++] = *str++;
        while (isdigit(*str) && len < ENV_VAR_NAME_MAX - 1)
        {
            result[len++] = *str++;
        }
    }
    if (len >= ENV_VAR_NAME_MAX - 1 && (isdigit(*str) || *str == '.'))
    {
        Read_state_set_error(state, "Exceeded maximum token length");
        return str;
    }
    assert(len < ENV_VAR_NAME_MAX);
    result[len] = '\0';
    if (result[0] == '0' && !(result[1] == '\0' || result[1] == '.'))
    {
        Read_state_set_error(state, "Leading zeros found");
        return str;
    }
    return str;
}


static char* get_str_token(char* str, char* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    char* end_str = "'";
    if (string_has_prefix(str, "\\\""))
    {
        end_str = "\\\"";
    }
    result[0] = *str++;
    int i = 1;
    while (!string_has_prefix(str, end_str))
    {
        if (i >= ENV_VAR_NAME_MAX + 1) // + 1 includes compensation for \"
        {
            Read_state_set_error(state, "Exceeded maximum token length");
            return str;
        }
        result[i] = *str++;
        ++i;
    }
    strcpy(&result[i], end_str);
    str += strlen(end_str);
    return str;
}


static char* get_var_token(char* str, char* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    int len = strspn(str, ENV_VAR_CHARS);
    if (len >= ENV_VAR_NAME_MAX)
    {
        Read_state_set_error(state, "Exceeded maximum token length");
        return str;
    }
    strncpy(result, str, len);
    result[len] = '\0';
    return str + len;
}


static char* get_op_token(char* str, char* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    static const char op_chars[] = "!=<>+-*/%^|&";
    int len = strspn(str, op_chars);
    if (len >= ENV_VAR_NAME_MAX)
    {
        Read_state_set_error(state, "Exceeded maximum token length");
        return str;
    }
    strncpy(result, str, len);
    result[len] = '\0';
    return str + len;
}


static bool op_eq(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    assert(op1->type > VALUE_TYPE_NONE);
    assert(op2->type > VALUE_TYPE_NONE);
    if (state->error)
    {
        return false;
    }
    if (op1->type == op2->type)
    {
        res->type = VALUE_TYPE_BOOL;
        switch(op1->type)
        {
            case VALUE_TYPE_BOOL:
            {
                res->value.bool_type = op1->value.bool_type ==
                                       op2->value.bool_type;
            } break;
            case VALUE_TYPE_INT:
            {
                res->value.bool_type = op1->value.int_type ==
                                       op2->value.int_type;
            } break;
            case VALUE_TYPE_FLOAT:
            {
                res->value.bool_type = op1->value.float_type ==
                                       op2->value.float_type;
            } break;
            case VALUE_TYPE_STRING:
            {
                res->value.bool_type = string_eq(op1->value.string_type,
                                                 op2->value.string_type);
            } break;
            default:
                assert(false);
        }
        return true;
    }
    if (op1->type > op2->type)
    {
        Value* tmp = op1;
        op1 = op2;
        op2 = tmp;
    }
    res->type = VALUE_TYPE_BOOL;
    if (op1->type == VALUE_TYPE_BOOL)
    {
        Read_state_set_error(state, "Comparison between boolean and number");
        return false;
    }
    else if (op2->type == VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Comparison between string and non-string");
        return false;
    }
    assert(op1->type == VALUE_TYPE_INT);
    assert(op2->type == VALUE_TYPE_FLOAT);
    res->value.bool_type = op1->value.int_type == op2->value.float_type;
    return true;
}


static bool op_neq(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (op_eq(op1, op2, res, state))
    {
        assert(res->type == VALUE_TYPE_BOOL);
        res->value.bool_type = !res->value.bool_type;
        return true;
    }
    return false;
}


static bool op_leq(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (!op_lt(op1, op2, res, state))
    {
        return false;
    }
    if (res->value.bool_type)
    {
        return true;
    }
    return op_eq(op1, op2, res, state);
}


static bool op_geq(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    return op_leq(op2, op1, res, state);
}


static bool op_or(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type != VALUE_TYPE_BOOL || op2->type != VALUE_TYPE_BOOL)
    {
        Read_state_set_error(state, "Boolean OR with non-booleans");
        return false;
    }
    res->type = VALUE_TYPE_BOOL;
    res->value.bool_type = op1->value.bool_type || op2->value.bool_type;
    return true;
}


static bool op_and(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type != VALUE_TYPE_BOOL || op2->type != VALUE_TYPE_BOOL)
    {
        Read_state_set_error(state, "Boolean AND with non-booleans");
        return false;
    }
    res->type = VALUE_TYPE_BOOL;
    res->value.bool_type = op1->value.bool_type && op2->value.bool_type;
    return true;
}


static bool op_lt(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Ordinal comparison between non-numbers");
        return false;
    }
    if (op1->type == op2->type)
    {
        res->type = VALUE_TYPE_BOOL;
        switch(op1->type)
        {
            case VALUE_TYPE_INT:
            {
                res->value.bool_type = op1->value.int_type <
                                       op2->value.int_type;
            } break;
            case VALUE_TYPE_FLOAT:
            {
                res->value.bool_type = op1->value.float_type <
                                       op2->value.float_type;
            } break;
            default:
                assert(false);
        }
        return true;
    }
    res->type = VALUE_TYPE_BOOL;
    if (op1->type == VALUE_TYPE_INT)
    {
        assert(op2->type == VALUE_TYPE_FLOAT);
        res->value.bool_type = op1->value.int_type < op2->value.float_type;
    }
    else
    {
        assert(op1->type == VALUE_TYPE_FLOAT);
        assert(op2->type == VALUE_TYPE_INT);
        res->value.bool_type = op1->value.float_type < op2->value.int_type;
    }
    return true;
}


static bool op_gt(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    return op_lt(op2, op1, res, state);
}


static bool op_add(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Addition with non-numbers");
        return false;
    }
    if (op1->type == op2->type)
    {
        res->type = op1->type;
        switch(op1->type)
        {
            case VALUE_TYPE_INT:
            {
                res->value.int_type = op1->value.int_type +
                                      op2->value.int_type;
            } break;
            case VALUE_TYPE_FLOAT:
            {
                res->value.float_type = op1->value.float_type +
                                        op2->value.float_type;
            } break;
            default:
                assert(false);
        }
        return true;
    }
    if (op1->type > op2->type)
    {
        Value* tmp = op1;
        op1 = op2;
        op2 = tmp;
    }
    assert(op1->type == VALUE_TYPE_INT);
    assert(op2->type == VALUE_TYPE_FLOAT);
    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = op1->value.int_type +
                            op2->value.float_type;
    return true;
}


static bool op_sub(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Subtraction with non-numbers");
        return false;
    }
    if (op2->type == VALUE_TYPE_INT)
    {
        op2->value.int_type = -op2->value.int_type;
    }
    else if (op2->type == VALUE_TYPE_FLOAT)
    {
        op2->value.float_type = -op2->value.float_type;
    }
    else
    {
        assert(false);
    }
    return op_add(op1, op2, res, state);
}


static bool op_mul(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Multiplication with non-numbers");
        return false;
    }
    if (op1->type == op2->type)
    {
        res->type = op1->type;
        switch(op1->type)
        {
            case VALUE_TYPE_INT:
            {
                res->value.int_type = op1->value.int_type *
                                      op2->value.int_type;
            } break;
            case VALUE_TYPE_FLOAT:
            {
                res->value.float_type = op1->value.float_type *
                                        op2->value.float_type;
            } break;
            default:
                assert(false);
        }
        return true;
    }
    if (op1->type > op2->type)
    {
        Value* tmp = op1;
        op1 = op2;
        op2 = tmp;
    }
    assert(op1->type == VALUE_TYPE_INT);
    assert(op2->type == VALUE_TYPE_FLOAT);
    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = op1->value.int_type *
                            op2->value.float_type;
    return true;
}


static bool op_div(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Division with non-numbers");
        return false;
    }
    if ((op2->type == VALUE_TYPE_INT && op2->value.int_type == 0) ||
            (op2->type == VALUE_TYPE_FLOAT && op2->value.float_type == 0))
    {
        Read_state_set_error(state, "Division by zero");
        return false;
    }
    if (op1->type == VALUE_TYPE_INT && op2->type == VALUE_TYPE_INT &&
            op1->value.int_type % op2->value.int_type == 0)
    {
        res->type = VALUE_TYPE_INT;
        res->value.int_type = op1->value.int_type / op2->value.int_type;
        return true;
    }
    if (op2->type == VALUE_TYPE_INT)
    {
        op2->type = VALUE_TYPE_FLOAT;
        op2->value.float_type = op2->value.int_type;
    }
    op2->value.float_type = 1.0 / op2->value.float_type;
    return op_mul(op1, op2, res, state);
}


static bool op_mod(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Modulo with non-numbers");
        return false;
    }
    if ((op2->type == VALUE_TYPE_INT && op2->value.int_type == 0) ||
            (op2->type == VALUE_TYPE_FLOAT && op2->value.float_type == 0))
    {
        Read_state_set_error(state, "Modulo by zero");
        return false;
    }
    if (op1->type == VALUE_TYPE_INT && op2->type == VALUE_TYPE_INT)
    {
        res->type = VALUE_TYPE_INT;
        res->value.int_type = op1->value.int_type % op2->value.int_type;
        if (res->value.int_type < 0)
        {
            res->value.int_type += op2->value.int_type;
        }
        return true;
    }
    double dividend = NAN;
    if (op1->type == VALUE_TYPE_INT)
    {
        dividend = op1->value.int_type;
    }
    else
    {
        assert(op1->type == VALUE_TYPE_FLOAT);
        dividend = op1->value.float_type;
    }
    double divisor = NAN;
    if (op2->type == VALUE_TYPE_INT)
    {
        divisor = op2->value.int_type;
    }
    else
    {
        assert(op2->type == VALUE_TYPE_FLOAT);
        divisor = op2->value.float_type;
    }
    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = fmod(dividend, divisor);
    if (res->value.float_type < 0)
    {
        res->value.float_type += divisor;
    }
    return true;
}


static bool op_pow(Value* op1, Value* op2, Value* res, Read_state* state)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL ||
            op1->type >= VALUE_TYPE_STRING || op2->type >= VALUE_TYPE_STRING)
    {
        Read_state_set_error(state, "Power with non-numbers");
        return false;
    }
    if (op1->type == VALUE_TYPE_INT && op2->type == VALUE_TYPE_INT &&
            op2->value.int_type >= 0)
    {
        res->type = VALUE_TYPE_INT;
        res->value.int_type = powi(op1->value.int_type, op2->value.int_type);
        return true;
    }
    double base = NAN;
    if (op1->type == VALUE_TYPE_INT)
    {
        base = op1->value.int_type;
    }
    else
    {
        assert(op1->type == VALUE_TYPE_FLOAT);
        base = op1->value.float_type;
    }
    double exp = NAN;
    if (op2->type == VALUE_TYPE_INT)
    {
        exp = op2->value.int_type;
    }
    else
    {
        assert(op2->type == VALUE_TYPE_FLOAT);
        exp = op2->value.float_type;
    }
    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = pow(base, exp);
    return true;
}


static bool func_ts(Value* args, Value* res, Read_state* state)
{
    assert(args != NULL);
    assert(res != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    res->type = VALUE_TYPE_TIMESTAMP;
    Reltime_init(&res->value.Timestamp_type);
    if (args[0].type == VALUE_TYPE_NONE)
    {
        return true;
    }
    else if (args[0].type == VALUE_TYPE_TIMESTAMP)
    {
        Value_copy(res, &args[0]);
        return true;
    }
    else if (args[0].type == VALUE_TYPE_INT)
    {
        Reltime_set(&res->value.Timestamp_type, args[0].value.int_type, 0);
    }
    else if (args[0].type == VALUE_TYPE_FLOAT)
    {
        double beats = floor(args[0].value.float_type);
        Reltime_set(&res->value.Timestamp_type, beats,
                    (args[0].value.float_type - beats) * KQT_RELTIME_BEAT);
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Read_state_set_error(state, "Invalid beat type");
        return false;
    }
    if (args[1].type == VALUE_TYPE_NONE)
    {
        return true;
    }
    else if (args[1].type == VALUE_TYPE_INT)
    {
        if (args[1].value.int_type < 0 ||
                args[1].value.int_type >= KQT_RELTIME_BEAT)
        {
            res->type = VALUE_TYPE_NONE;
            Read_state_set_error(state, "Invalid beat value");
            return false;
        }
        Reltime_add(&res->value.Timestamp_type, &res->value.Timestamp_type,
                    Reltime_set(RELTIME_AUTO, 0, args[1].value.int_type));
    }
    else if (args[1].type == VALUE_TYPE_FLOAT)
    {
        if (args[1].value.float_type < 0 ||
                args[1].value.float_type >= KQT_RELTIME_BEAT)
        {
            res->type = VALUE_TYPE_NONE;
            Read_state_set_error(state, "Invalid beat value");
            return false;
        }
        Reltime_add(&res->value.Timestamp_type, &res->value.Timestamp_type,
                    Reltime_set(RELTIME_AUTO, 0, args[1].value.float_type));
    }
    else
    {
        res->type = VALUE_TYPE_NONE;
        Read_state_set_error(state, "Invalid remainder type");
        return false;
    }
    return true;
}


