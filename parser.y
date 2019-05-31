%{
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include "ast.hpp"

#define YYDEBUG 1
#define YYINITDEPTH 10000

int yylex(void);
void yyerror(const char *);

extern ASTNode* astRoot;
%}

%error-verbose
// %glr-parser
/* NOTE: You may use the %glr-parser directive, which may allow your parser to
work even with some shift/reduce conflicts remianing. */

/* WRITEME: Copy your token and precedence specifiers from Project 4 here. */
%token T_PRINT
%token T_RETURN
%token T_IF
%token T_ELSE T_WHILE T_NEW T_INTEGER T_BOOLEAN
%token T_NONE
%token T_EQUALS
%token T_AND T_OR
%token T_NOT T_UNARYMINUS
%token T_TRUE T_FALSE T_LITERAL
%token T_EXTENDS
%token T_DO
%token T_ARROW
%token T_GTET
%token T_ID


%left T_OR
%left T_AND
%left '>' T_GTET T_EQUALS
%left '+' '-'
%left '*' '/'
%precedence T_NOT T_UNARYMINUS
/* WRITEME: Copy your type specifiers from Project 4 here. */
%type <program_ptr> Start
%type <class_list_ptr> Classes
%type <class_ptr> Class
%type <identifier_ptr> T_ID
%type <declaration_list_ptr> Members Declarations
%type <method_list_ptr> Methods
%type <method_ptr> Method
%type <parameter_list_ptr> Parameters
%type <type_ptr> type ReturnType
%type <methodbody_ptr> Body
%type <statement_list_ptr> Statements StatementList
%type <statement_ptr> Statement
%type <returnstatement_ptr> ReturnStatement
%type <parameter_ptr> Parameter
%type <declaration_ptr> Declaration Member
%type <identifier_list_ptr> idList
%type <expression_ptr> Expression Argument
%type <assignment_ptr> Assignment
%type <methodcall_ptr> MethodCall
%type <ifelse_ptr> IfElse
%type <while_ptr> While
%type <dowhile_ptr> doWhile
%type <print_ptr> Print
%type <expression_list_ptr> Arguments ArgumentList
%type <integerliteral_ptr> T_LITERAL
%%

/* WRITEME: This rule is a placeholder. Replace it with your grammar
rules and actions from Project 4. */

Start : Classes  {astRoot = new ProgramNode($1);}
      ;
Classes : Classes Class {$$ = $1;$$->push_back($2);}
        |Class  {$$ = new std::list<ClassNode*>();$$->push_back($1);}
        ;
Class : T_ID '{' Members Methods'}' { $$ = new ClassNode($1, NULL, $3, $4); }
      | T_ID T_EXTENDS T_ID '{' Members Methods'}'  { $$ = new ClassNode($1, $3, $5, $6); }
      ;
Members : Members Member {$$ = $1; $$->push_back($2);}
        | %empty { $$ = new std::list<DeclarationNode*>(); }
        ;
Member : type idList ';'    {$$ = new DeclarationNode($1,$2);}
       ;

Methods : Method Methods {$$ = $2 ; $$->push_front($1);}
        | %empty {$$ = new std::list<MethodNode*>();}
        ;

Method : T_ID '(' Parameters ')' T_ARROW ReturnType '{' Body '}' {$$ = new MethodNode($1,$3,$6,$8); }
        |T_ID '('')' T_ARROW ReturnType '{' Body '}' {$$ = new MethodNode($1,NULL,$5,$7);}
           ;
Parameters : Parameters ',' Parameter {$$ = $1; $$->push_back($3);}
              | Parameter {$$ = new std::list<ParameterNode*>(); $$->push_back($1);}
              ;
Parameter : type T_ID  {$$ = new ParameterNode($1,$2);}
          ;
Body : Declarations Statements ReturnStatement {$$ = new MethodBodyNode($1,$2,$3);}
     ;
Declarations : Declarations Declaration ';' {$$ = $1; $$->push_back($2);}
              |%empty { $$ = new std::list<DeclarationNode*>();}
             ;
Declaration : type idList {$$ = new DeclarationNode($1,$2);}
            ;
StatementList : Statement Statements {$$ = $2 ;$$->push_front($1);}
                ;

Statements : Statement Statements {$$ = $2; $$->push_front($1);}
           | %empty {$$ = new std::list<StatementNode*>();}
           ;
Statement : Assignment ';' {$$ = $1;}
          | MethodCall ';' {$$ = new CallNode($1);}
          | IfElse         {$$ = $1;}
          | While          {$$ = $1;}
          | doWhile ';'    {$$ = $1;}
          | Print ';'      {$$ = $1;}
          ;
ReturnStatement : T_RETURN Expression ';'    {$$ = new ReturnStatementNode($2);}
                |%empty                      {$$ = NULL;}
                ;
idList : T_ID ',' idList  {$$ = $3; new std::list<IdentifierNode*>(); $$->push_front($1);}
       | T_ID           {$$= new std::list<IdentifierNode*>(); $$->push_back($1);}
       ;
Assignment : T_ID '=' Expression         {$$ = new AssignmentNode($1,NULL,$3);}
           | T_ID '.' T_ID '=' Expression { $$ = new AssignmentNode($1,$3,$5);}
           ;
MethodCall : T_ID '(' Arguments ')'      {$$ = new MethodCallNode($1,NULL,$3);}
           |	T_ID '.' T_ID '(' Arguments ')' {$$ = new MethodCallNode($1,$3,$5);}
           ;
IfElse : T_IF Expression '{' StatementList '}'  {$$= new IfElseNode($2,$4,NULL);}
       | T_IF Expression '{'  StatementList'}' T_ELSE '{'  StatementList'}'   {$$= new IfElseNode($2,$4,$8);}
       ;
While : T_WHILE Expression '{'  StatementList'}'   {$$ = new WhileNode($2,$4);}
      ;
doWhile : T_DO '{'  Statements '}' T_WHILE '(' Expression ')'  {$$ = new DoWhileNode($3,$7);}
        ;

Print : T_PRINT Expression  {$$ = new PrintNode ($2);}
      ;
type : T_ID {$$ = new ObjectTypeNode($1); }
     | T_BOOLEAN {$$ = new BooleanTypeNode();}
     | T_INTEGER   { $$ = new IntegerTypeNode(); }
     ;
ReturnType : T_ID  {$$ = new ObjectTypeNode($1); }
      | T_BOOLEAN  {$$ = new BooleanTypeNode();}
      | T_INTEGER  { $$ = new IntegerTypeNode(); }
      | T_NONE    { $$ = new NoneNode(); }
      ;
Expression  : Expression '+' Expression  {$$ = new PlusNode($1,$3);}
            |	Expression '-' Expression  {$$ = new MinusNode($1,$3);}
            |	Expression '*' Expression  {$$ = new TimesNode($1,$3);}
            |	Expression '/' Expression  {$$ = new DivideNode($1,$3);}
            |	Expression '>' Expression  {$$ = new GreaterNode($1,$3);}
            |	Expression T_GTET Expression  {$$ = new GreaterEqualNode($1,$3);}
            |	Expression T_AND Expression    {$$ = new AndNode($1,$3);}
            |	Expression T_OR Expression     {$$ = new OrNode($1,$3);}
            | Expression T_EQUALS Expression {$$ = new EqualNode($1,$3);}
            | T_NOT Expression               {$$ = new NotNode($2);}
            | '-' Expression %prec T_UNARYMINUS {$$ = new NegationNode($2);}
            | T_ID          {$$ = new VariableNode($1);}
            |	T_ID '.' T_ID {$$ = new MemberAccessNode($1,$3);}
            |	MethodCall    {$$ = $1;}
            |	'(' Expression ')' {$$ =  $2;}
            |	T_LITERAL          {$$ = $1;}
            |	T_TRUE     {$$ = new BooleanLiteralNode(new IntegerNode(1));}
            |	T_FALSE    {$$ = new BooleanLiteralNode(new IntegerNode(0));}
            |	T_NEW T_ID {$$ = new NewNode($2,NULL);}
            |	T_NEW T_ID '(' Arguments ')' {$$ = new NewNode($2,$4);}
            ;

Arguments	:	ArgumentList { $$= $1 ;}
          |	%empty      {$$ = new std::list<ExpressionNode*>();}
          ;
ArgumentList : Argument ',' ArgumentList {$$ = $3; $$->push_front($1);}
           |	Argument   {$$= new std::list<ExpressionNode*>(); $$->push_back($1);}
           ;
Argument : Expression {$$ = $1;}
         ;

%%

extern int yylineno;

void yyerror(const char *s) {
fprintf(stderr, "%s at line %d\n", s, yylineno);
exit(1);
}
