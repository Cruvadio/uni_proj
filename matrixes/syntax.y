
PROGRAM -> DECLARATION PROCESS EOF
DECLARATION -> "declare:" DECLARE | DECLARATION DECLARE 
DECLARE -> TYPE: ID | DECLARE, ID; 
TYPE -> "matrix" | "vector" | "rational" | "float" | "integer"
ID -> VAR | VAR ([NUM | STRING | FLOAT | RATIO]);
VAR -> [a-z | A-Z | _ ] | WORD [0-9]
PROCESS -> "process:" OPS
OPS -> OP | OPS OP
OP -> [ASSIGN | EXPRESSION];
ASSIGN -> ID = EXPRESSION
EXPRESSION -> EXPR | EXPR [+, -] EXPRESSION
EXPR -> ACTION | ACTION [*, /, ^] EXPR
EXPR2 -> OBJ | (EXPRESSION) | (ASSIGN)
OBJ -> ID | NUM | RATIO | FLOAT | VAR'['[NUM (, NUM)]']'
ACTION -> EXPR2 : [FUNC | FUNC (STRING)] | EXPR2

