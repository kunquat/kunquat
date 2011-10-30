

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <Env_var.h>
#include <Event_common.h>
#include <Event_general.h>
#include <Event_general_cond.h>
#include <File_base.h>
#include <General_state.h>
#include <math_common.h>
#include <Real.h>
#include <Reltime.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


static bool evaluate_cond(char* str, Environment* env, Read_state* state);
static char* get_token(char* str, char* result, Read_state* state);
static char* get_num_token(char* str, char* result, Read_state* state);
static char* get_var_token(char* str, char* result, Read_state* state);
static char* get_op_token(char* str, char* result, Read_state* state);


typedef enum
{
    VALUE_TYPE_NONE = 0,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_REAL,
    VALUE_TYPE_TIMESTAMP,
} Value_type;


typedef struct Value
{
    Value_type type;
    union
    {
        bool bool_type;
        int64_t int_type;
        double float_type;
        Real Real_type;
        Reltime Timestamp_type;
    } value;
} Value;


#define VALUE_AUTO (&(Value){ .type = VALUE_TYPE_NONE })


static bool Value_from_token(Value* val, char* token, Environment* env);

static void Value_print(Value* val);


typedef bool (*Op_func)(Value* op1, Value* op2, Value* res);


typedef struct Operator
{
    char* name;
    int preced;
    Op_func func;
} Operator;


#define OPERATOR_AUTO (&(Operator){ .name = NULL, .func = NULL })


static bool Operator_from_token(Operator* op, char* token);

static void Operator_print(Operator* op);


static bool op_neq(Value* op1, Value* op2, Value* res);
static bool op_leq(Value* op1, Value* op2, Value* res);
static bool op_geq(Value* op1, Value* op2, Value* res);
static bool op_or(Value* op1, Value* op2, Value* res);
static bool op_and(Value* op1, Value* op2, Value* res);
static bool op_eq(Value* op1, Value* op2, Value* res);
//static bool op_not(Value* op1, Value* op2, Value* res);
static bool op_lt(Value* op1, Value* op2, Value* res);
static bool op_gt(Value* op1, Value* op2, Value* res);
static bool op_add(Value* op1, Value* op2, Value* res);
static bool op_sub(Value* op1, Value* op2, Value* res);
static bool op_mul(Value* op1, Value* op2, Value* res);
static bool op_div(Value* op1, Value* op2, Value* res);
static bool op_mod(Value* op1, Value* op2, Value* res);
static bool op_pow(Value* op1, Value* op2, Value* res);


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


static Event_field_desc cond_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_general,
                         EVENT_GENERAL_COND,
                         cond);


bool Event_general_cond_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, cond_desc, data, state);
    if (state->error)
    {
        return false;
    }
    state = READ_STATE_AUTO;
    bool cond = evaluate_cond(data[0].field.string_type, gstate->env, state);
    if (!state->error)
    {
        fprintf(stderr, "success!\n");
        gstate->evaluated_cond = cond;
    }
    return true;
}


#define STACK_SIZE 32


static bool evaluate_expr(char* str, Environment* env, Read_state* state,
                          Value* val_stack, int vsi,
                          Operator* op_stack, int osi,
                          Value* res, int depth);


static bool evaluate_cond(char* str, Environment* env, Read_state* state)
{
    assert(str != NULL);
    assert(str[0] == '"');
    assert(env != NULL);
    ++str;
    Value val_stack[STACK_SIZE] = { { .type = VALUE_TYPE_NONE } };
    Operator op_stack[STACK_SIZE] = { { .name = NULL } };
    Value* result = VALUE_AUTO;
    if (!evaluate_expr(str, env, state, val_stack, 0, op_stack, 0, result, 0))
    {
        fprintf(stderr, "\n");
        if (!state->error)
        {
            Read_state_set_error(state, "Evaluation failed.");
        }
        return false;
    }
    fprintf(stderr, "\n");
    if (result->type != VALUE_TYPE_BOOL)
    {
        Read_state_set_error(state, "Expression is not of boolean type.");
        return false;
    }
    Value_print(result);
    fprintf(stderr, "\n");
    return result->value.bool_type;
}


static bool handle_unary(Value* val, bool found_not, bool found_minus,
                         Read_state* state);


#define check_stack(si) if (true)                           \
    {                                                       \
        if ((si) >= STACK_SIZE)                             \
        {                                                   \
            assert((si) == STACK_SIZE);                     \
            Read_state_set_error(state, "Stack overflow");  \
            return false;                                   \
        }                                                   \
    } else (void)0

static bool evaluate_expr(char* str, Environment* env, Read_state* state,
                          Value* val_stack, int vsi,
                          Operator* op_stack, int osi,
                          Value* res, int depth)
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
    assert(res != NULL);
    assert(depth >= 0);
    if (state->error)
    {
        return false;
    }
    if (depth >= STACK_SIZE)
    {
        Read_state_set_error(state, "Maximum recursion depth exceeded");
        return false;
    }
    int orig_vsi = vsi;
    int orig_osi = osi;
    char token[ENV_VAR_NAME_MAX] = "";
    bool expect_operand = true;
    bool found_not = false;
    bool found_minus = false;
    str = get_token(str, token, state);
    while (!state->error && !string_eq(token, "") &&
            !string_eq(token, ")"))
    {
        Value* operand = VALUE_AUTO;
        Operator* op = OPERATOR_AUTO;
        if (string_eq(token, "\"") || string_eq(token, ")"))
        {
            if (orig_vsi >= vsi)
            {
                assert(orig_vsi == vsi);
                Read_state_set_error(state, "Empty parentheses");
                return false;
            }
            if ((depth == 0) != string_eq(token, "\""))
            {
                Read_state_set_error(state, "Unmatched %s parenthesis",
                                     depth == 0 ? "left" : "right");
                return false;
            }
            break;
        }
        else if (string_eq(token, "("))
        {
            if (!expect_operand)
            {
                Read_state_set_error(state, "Unexpected operand");
                return false;
            }
            check_stack(vsi);
            if (!evaluate_expr(str, env, state, val_stack, vsi,
                               op_stack, osi, operand, depth + 1))
            {
                return false;
            }
            assert(operand->type != VALUE_TYPE_NONE);
            if (!handle_unary(operand, found_not, found_minus, state))
            {
                return false;
            }
            found_not = found_minus = false;
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
        }
        else if (Value_from_token(operand, token, env))
        {
            if (!expect_operand)
            {
                Read_state_set_error(state, "Unexpected operand");
                return false;
            }
            assert(operand->type != VALUE_TYPE_NONE);
            if (!handle_unary(operand, found_not, found_minus, state))
            {
                return false;
            }
            found_not = found_minus = false;
            check_stack(vsi);
            memcpy(&val_stack[vsi], operand, sizeof(Value));
            ++vsi;
            expect_operand = false;
            Value_print(operand);
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
                    return false;
                }
                str = get_token(str, token, state);
                continue;
            }
            if (string_eq(op->name, "!"))
            {
                Read_state_set_error(state, "Unexpected boolean not");
                return false;
            }
            while (osi > orig_osi && op->preced <= op_stack[osi - 1].preced)
            {
                Operator* top = &op_stack[osi - 1];
                assert(top->name != NULL);
                assert(top->func != NULL);
                if (vsi < 2)
                {
                    Read_state_set_error(state, "Not enough operands");
                    return false;
                }
                Value* result = VALUE_AUTO;
                if (!top->func(&val_stack[vsi - 2], &val_stack[vsi - 1],
                               result))
                {
                    Read_state_set_error(state, "Operator error");
                    return false;
                }
                Operator_print(top);
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
            return false;
        }
        str = get_token(str, token, state);
    }
    if (state->error)
    {
        return false;
    }
    while (osi > orig_osi)
    {
        Operator* top = &op_stack[osi - 1];
        assert(top->name != NULL);
        assert(top->func != NULL);
        if (vsi < 2)
        {
            Read_state_set_error(state, "Not enough operands");
            return false;
        }
        Value* result = VALUE_AUTO;
        if (!top->func(&val_stack[vsi - 2], &val_stack[vsi - 1],
                       result))
        {
            Read_state_set_error(state, "Operator error");
            return false;
        }
        Operator_print(top);
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
    return true;
}

#undef check_stack


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
    Read_state_set_error(state, "Non-number operand for unary minus");
    return false;
}


static bool Value_from_token(Value* val, char* token, Environment* env)
{
    assert(val != NULL);
    assert(token != NULL);
    assert(env != NULL);
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
    else if (string_eq(token, "true") || string_eq(token, "false"))
    {
        val->type = VALUE_TYPE_BOOL;
        val->value.bool_type = string_eq(token, "true");
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


static void Operator_print(Operator* op)
{
    assert(op != NULL);
    assert(op->name != NULL);
    fprintf(stderr, "%s ", op->name);
    return;
}


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


static bool op_neq(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op_eq(op1, op2, res))
    {
        assert(res->type == VALUE_TYPE_BOOL);
        res->value.bool_type = !res->value.bool_type;
        return true;
    }
    return false;
}


static bool op_leq(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (!op_lt(op1, op2, res))
    {
        return false;
    }
    if (res->value.bool_type)
    {
        return true;
    }
    return op_eq(op1, op2, res);
}


static bool op_geq(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    return op_leq(op2, op1, res);
}


static bool op_or(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type != VALUE_TYPE_BOOL || op2->type != VALUE_TYPE_BOOL)
    {
        return false;
    }
    res->type = VALUE_TYPE_BOOL;
    res->value.bool_type = op1->value.bool_type || op2->value.bool_type;
    return true;
}


static bool op_and(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type != VALUE_TYPE_BOOL || op2->type != VALUE_TYPE_BOOL)
    {
        return false;
    }
    res->type = VALUE_TYPE_BOOL;
    res->value.bool_type = op1->value.bool_type && op2->value.bool_type;
    return true;
}


static bool op_eq(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type == VALUE_TYPE_NONE || op2->type == VALUE_TYPE_NONE)
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
        res->value.bool_type = false;
        return true;
    }
    assert(op1->type == VALUE_TYPE_INT);
    assert(op2->type == VALUE_TYPE_FLOAT);
    res->value.bool_type = op1->value.int_type == op2->value.float_type;
    return true;
}


#if 0
static bool op_not(Value* op1, Value* op2, Value* res)
{
}
#endif


static bool op_lt(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
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


static bool op_gt(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    return op_lt(op2, op1, res);
}


static bool op_add(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
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


static bool op_sub(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
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
    return op_add(op1, op2, res);
}


static bool op_mul(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
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


static bool op_div(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
        return false;
    }
    if ((op2->type == VALUE_TYPE_INT && op2->value.int_type == 0) ||
            (op2->type == VALUE_TYPE_FLOAT && op2->value.float_type == 0))
    {
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
    return op_mul(op1, op2, res);
}


static bool op_mod(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
        return false;
    }
    if (op1->type == VALUE_TYPE_INT && op2->type == VALUE_TYPE_INT)
    {
        if (op2->value.int_type == 0)
        {
            return false;
        }
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
    if (divisor == 0)
    {
        return false;
    }
    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = fmod(dividend, divisor);
    if (res->value.float_type < 0)
    {
        res->value.float_type += divisor;
    }
    return true;
}


static bool op_pow(Value* op1, Value* op2, Value* res)
{
    assert(op1 != NULL);
    assert(op2 != NULL);
    assert(res != NULL);
    if (op1->type <= VALUE_TYPE_BOOL || op2->type <= VALUE_TYPE_BOOL)
    {
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
        base = op1->value.float_type;
        assert(op1->type == VALUE_TYPE_FLOAT);
    }
    double exp = NAN;
    if (op2->type == VALUE_TYPE_INT)
    {
        exp = op2->value.int_type;
    }
    else
    {
        exp = op2->value.float_type;
        assert(op2->type == VALUE_TYPE_FLOAT);
    }
    res->type = VALUE_TYPE_FLOAT;
    res->value.float_type = pow(base, exp);
    return true;
}


