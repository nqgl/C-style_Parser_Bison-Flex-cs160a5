#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
    switch (code) {
        case undefined_variable:
        std::cerr << "Undefined variable." << std::endl;
        break;
        case undefined_method:
        std::cerr << "Method does not exist." << std::endl;
        break;
        case undefined_class:
        std::cerr << "Class does not exist." << std::endl;
        break;
        case undefined_member:
        std::cerr << "Class member does not exist." << std::endl;
        break;
        case not_object:
        std::cerr << "Variable is not an object." << std::endl;
        break;
        case expression_type_mismatch:
        std::cerr << "Expression types do not match." << std::endl;
        break;
        case argument_number_mismatch:
        std::cerr << "Method called with incorrect number of arguments." << std::endl;
        break;
        case argument_type_mismatch:
        std::cerr << "Method called with argument of incorrect type." << std::endl;
        break;
        case while_predicate_type_mismatch:
        std::cerr << "Predicate of while loop is not boolean." << std::endl;
        break;
        case do_while_predicate_type_mismatch:
        std::cerr << "Predicate of do while loop is not boolean." << std::endl;
        break;
        case if_predicate_type_mismatch:
        std::cerr << "Predicate of if statement is not boolean." << std::endl;
        break;
        case assignment_type_mismatch:
        std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
        break;
        case return_type_mismatch:
        std::cerr << "Return statement type does not match declared return type." << std::endl;
        break;
        case constructor_returns_type:
        std::cerr << "Class constructor returns a value." << std::endl;
        break;
        case no_main_class:
        std::cerr << "The \"Main\" class was not found." << std::endl;
        break;
        case main_class_members_present:
        std::cerr << "The \"Main\" class has members." << std::endl;
        break;
        case no_main_method:
        std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
        break;
        case main_method_incorrect_signature:
        std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
        break;
    }
    exit(1);
}

// HELPER functions

ClassInfo getClassInfo(std::string& identifier, TypeCheck* scope) {
    if (scope->classTable->count(identifier) != 0){
        return (*(scope->classTable))[identifier];
    }
    else {
        typeError(undefined_class);
    }
}


VariableInfo getVariableInfoFromClassMember(std::string& classname, std::string& identifier, TypeCheck* scope){
    ClassInfo classInfo = getClassInfo(classname, scope);
    if (classInfo.members->count(identifier) != 0) {
        return classInfo.members->at(identifier);
    }
    else if (classInfo.superClassName != ""){
        if (scope->classTable->count(classInfo.superClassName) != 0){
            return getVariableInfoFromClassMember(classInfo.superClassName, identifier, scope);
        }
        else {
            CompoundType failuretype {bt_none, "failure"};
            VariableInfo varinfo {failuretype, -1, -1};
            return varinfo;
        }
    } else {
            CompoundType failuretype {bt_none, "failure"};
            VariableInfo varinfo {failuretype, -1, -1};
            return varinfo;
    }
}

VariableInfo getVariableInfo(std::string& identifier, TypeCheck* scope){
        // looks for, in this order:
        // local variables
        // parameters (how to find their name? not in methodinfo)
        // member of class (or superclass (check recursively probably))
        // try search in currentvariabletable
    if (scope->currentVariableTable->count(identifier) != 0){
                // searches both locals and parameters,
                // because both are stored in the same variable table
        return scope->currentVariableTable->at(identifier);
    }
    else {
        VariableInfo checkInfo = getVariableInfoFromClassMember(scope->currentClassName, identifier, scope);
        if (checkInfo.type.baseType == bt_none && checkInfo.type.objectClassName == "failure") {
            typeError(undefined_variable);
        }
        return checkInfo;
    }
}




MethodInfo getMethodInfoFromClass(std::string& classname, std::string& identifier, TypeCheck* scope){
    ClassInfo classInfo = getClassInfo(classname, scope);
    if (classInfo.methods->count(identifier) != 0) {
        return classInfo.methods->at(identifier);
    }
    else if (classInfo.superClassName != ""){
        if (scope->classTable->count(classInfo.superClassName) != 0){
            return getMethodInfoFromClass(classInfo.superClassName, identifier, scope);
        }
        else {
            typeError(undefined_method);
        }        
    }
    else {
            typeError(undefined_method);
    }
}



CompoundType getExpressionType(ExpressionNode* expression, TypeCheck* visitor){
    if (typeid(*expression) == typeid(PlusNode)
        || (typeid(*expression) == typeid(MinusNode))
        || (typeid(*expression) == typeid(TimesNode))
        || (typeid(*expression) == typeid(DivideNode))
        || (typeid(*expression) == typeid(NegationNode))
        || (typeid(*expression) == typeid(IntegerLiteralNode))){
            CompoundType intType = {bt_integer, ""};
            return intType;
    } else if (typeid(*expression) == typeid(GreaterNode)
        || (typeid(*expression) == typeid(GreaterEqualNode))
        || (typeid(*expression) == typeid(EqualNode))
        || (typeid(*expression) == typeid(AndNode))
        || (typeid(*expression) == typeid(OrNode))
        || (typeid(*expression) == typeid(NotNode))
        || (typeid(*expression) == typeid(BooleanLiteralNode))){
        CompoundType booleanType = {bt_boolean, ""};
        return booleanType;
    }
    else if (typeid(*expression) == typeid(VariableNode)){
        VariableNode* variable = dynamic_cast<VariableNode*>(expression);
        VariableInfo varInfo = getVariableInfo(variable->identifier->name, visitor);
        return varInfo.type;
    }
    else if (typeid(*expression) == typeid(MethodCallNode)) {
        MethodCallNode* methodCall = dynamic_cast<MethodCallNode*>(expression);
        std::string callertype;
        std::string calledname;
        if (methodCall->identifier_2 == NULL){
            // if first id doesn't exist
            callertype = visitor->currentClassName;
            calledname = methodCall->identifier_1->name;
        }
        else {
            VariableInfo objectInfo = getVariableInfo(methodCall->identifier_1->name, visitor);
            if (objectInfo.type.baseType != bt_object){
                // we would be unable to access a method of it, thus error in source code
                typeError(not_object);
            }
            callertype = objectInfo.type.objectClassName;
            calledname = methodCall->identifier_2->name;
        }
        MethodInfo methodCalled = getMethodInfoFromClass(callertype, calledname, visitor);
        // go thru methodCall list
        std::list<ExpressionNode*>::const_iterator i_args = methodCall->expression_list->begin();
        std::list<CompoundType>::const_iterator i_params = methodCalled.parameters->begin();
        CompoundType argType;
        while(i_params != methodCalled.parameters->end() && i_args != methodCall->expression_list->end()){
            argType = getExpressionType(*i_args, visitor);
            if ((argType.baseType != i_params->baseType)
                || (argType.objectClassName != i_params->objectClassName)){
                typeError(argument_type_mismatch);
            }
            i_args++;
            i_params++;
        }
        if (i_params != methodCalled.parameters->end() || i_args != methodCall->expression_list->end()){
            typeError(argument_number_mismatch);
        }
        return methodCalled.returnType;
    }
    else if (typeid(*expression) == typeid(MemberAccessNode)) {
        MemberAccessNode* memberAccess = dynamic_cast<MemberAccessNode*>(expression);
        VariableInfo objectInfo = getVariableInfo(memberAccess->identifier_1->name, visitor);
        if (objectInfo.type.baseType != bt_object){
            // we would be unable to access a member of it, thus error in source code
            typeError(not_object);
        }
        VariableInfo checkInfo = getVariableInfoFromClassMember(objectInfo.type.objectClassName, memberAccess->identifier_2->name, visitor);
        if (checkInfo.type.baseType == bt_none && checkInfo.type.objectClassName == "failure"){
            typeError(undefined_member);
        }
        else {
            return checkInfo.type;
        }
    }
    else if (typeid(*expression) == typeid(NewNode)) {
        // todo
        // assuming that there always must be a constructor defined
        NewNode* newExpression = dynamic_cast<NewNode*>(expression);
        if (visitor->classTable->count(newExpression->identifier->name) == 0){
            typeError(undefined_class);
        }
        ClassInfo typeClassInfo = visitor->classTable->at(newExpression->identifier->name);
        if (typeClassInfo.methods->count(newExpression->identifier->name) == 0){
            typeError(undefined_method);
        }
        else{
            MethodInfo constructorInfo = typeClassInfo.methods->at(newExpression->identifier->name);
            // go thru expression list
            std::list<ExpressionNode*>::const_iterator i_args = newExpression->expression_list->begin();
            std::list<CompoundType>::const_iterator i_params = constructorInfo.parameters->begin();
            CompoundType argType;
            while(i_params != constructorInfo.parameters->end() && i_args != newExpression->expression_list->end()){
                argType = getExpressionType(*i_args, visitor);
                if ((argType.baseType != i_params->baseType)
                    || (argType.objectClassName != i_params->objectClassName)){
                    typeError(argument_type_mismatch);
                }
                i_args++;
                i_params++;
            }
            if (i_params != constructorInfo.parameters->end() || i_args != newExpression->expression_list->end()){
                typeError(argument_number_mismatch);
            }
        }
        CompoundType newType = {bt_object, newExpression->identifier->name};
        return newType;
    }
}

// maybe a function to look up a name in current scope
// unclear whether it should be a different function for looking up a methodinfo vs a vaiableinfo vs a classinfo


CompoundType compoundFromTypeNode(TypeNode* node){
    CompoundType nodetype;
    if (typeid(*node) == typeid(IntegerTypeNode)){
        nodetype.baseType = bt_integer;
        nodetype.objectClassName = "";
    }
    else if (typeid(*node) == typeid(BooleanTypeNode)){
        nodetype.baseType = bt_boolean;
        nodetype.objectClassName = "";
    }
    else if (typeid(*node) == typeid(NoneNode)){
        nodetype.baseType = bt_none;
        nodetype.objectClassName = "";
    }
    else if (typeid(*node) == typeid(ObjectTypeNode)){
        ObjectTypeNode* objectNode = dynamic_cast<ObjectTypeNode*>(node);
        nodetype.baseType = bt_object;
        nodetype.objectClassName = objectNode->identifier->name;
    }
    return nodetype;
}

bool areSameCompoundType(CompoundType& t1, CompoundType& t2){
    return (t1.baseType == t2.baseType) && (t1.objectClassName == t2.objectClassName);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
    // WRITEME: Replace with code if necessary
    this->classTable = new ClassTable();
    node->visit_children(this);
    if (classTable->count("Main")==0){
        typeError(no_main_class);
    }
    ClassInfo main_class = (*(classTable))["Main"];
    if (main_class.methods->count("main")==0) {
        typeError(no_main_method);
    }
    if (!main_class.members->empty()){
        typeError(main_class_members_present);
    }
    MethodInfo main_method = (*(classTable->at("Main").methods))["main"];
    if (main_method.returnType.baseType != bt_none || main_method.parameters->size() != 0){
        typeError(main_method_incorrect_signature);
    }
}


void TypeCheck::visitClassNode(ClassNode* node) {
    // WRITEME: Replace with code if necessary
    ClassInfo classInfo;
    classInfo.methods = new MethodTable();
    classInfo.members = new VariableTable();
    currentLocalOffset = 0;
    if (node->identifier_2 != NULL) {
        classInfo.superClassName = node->identifier_2->name; // probably need to
        if (classTable->count(classInfo.superClassName) == 0){
            typeError(undefined_class);
        }
    }
    else {
        classInfo.superClassName = "";
    }
    (*(this->classTable))[node->identifier_1->name] = classInfo;
    this->currentMethodTable = classInfo.methods;
    this->currentVariableTable = classInfo.members;

    this->currentClassName = node->identifier_1->name;
    node->visit_children(this);

    classInfo.membersSize = classInfo.members->size()*4; //TA magic

/*    if (this->classTable->count(currentClassName)==0){ // if constructor returns a type, throw error? this doesn't do that
        typeError(undefined_class);                     // I think it's also checking the wrong condition?
    }*/
}


// errors to catch: constructor_returns_type and return_type_mismatch
void TypeCheck::visitMethodNode(MethodNode* node) {
    // setup methodinfo entry
    MethodInfo methodInfo;
    methodInfo.variables = new VariableTable();
    methodInfo.returnType = compoundFromTypeNode(node->type);
    methodInfo.parameters = new std::list<CompoundType>();
    // prime visitor for next visits
    this->currentLocalOffset = -4;
    this->currentParameterOffset = 12;
    this->currentVariableTable = methodInfo.variables;

    // set up methodinfo->parameters and put parameters into variable Table
    if (node->parameter_list != NULL) {
        for (ParameterNode *p : *(node->parameter_list)) {

            VariableInfo parameterInfo; // var table info
            CompoundType parameterType = compoundFromTypeNode(p->type); // param list info

            methodInfo.parameters->push_back(parameterType);
            parameterInfo.offset = this->currentParameterOffset;
            this->currentParameterOffset += 4;
            if (parameterType.baseType == bt_object) {
                parameterInfo.size = 4;
            } else {
                parameterInfo.size = 4;
            }
            parameterInfo.type = parameterType;

            (*(methodInfo.variables))[p->identifier->name] = parameterInfo;
        }
    }
    node->visit_children(this);
    if (node->parameter_list != NULL) {
        methodInfo.localsSize = (methodInfo.variables->size() - node->parameter_list->size()) * 4; // reserve 4 for each pointer needed for each local
    } else {
        methodInfo.localsSize = methodInfo.variables->size() * 4; // reserve 4 for each pointer needed for each local

    }
    if (node->identifier->name == this->currentClassName){
    // if methodInfo is constructor
    // by if it has the same name as current class return statement is type of methodInfo
        if (methodInfo.returnType.baseType != bt_none){
            if ((*(this->currentMethodTable))[node->identifier->name].returnType.baseType != bt_none){
                typeError(constructor_returns_type);
            }
        }
    }

    // check return type
    CompoundType returnStatementType;
    if(node->methodbody->returnstatement == NULL){
         returnStatementType = {bt_none, ""};
    }
    else{
        returnStatementType = getExpressionType(node->methodbody->returnstatement->expression, this);
    }
    //typeError(undefined_variable); //testerror
    if (!areSameCompoundType(returnStatementType, methodInfo.returnType)){
        typeError(return_type_mismatch);
    }

    (*(this->currentMethodTable))[node->identifier->name] = methodInfo;
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
    // WRITEME: Replace with code if necessary
    // do we want to pass in the next methodbodyinfo by

    node->visit_children(this);
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
    // handled in visitMethodNode
    // WRITEME: Replace with code if necessary
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
    // WRITEME: Replace with code if necessary
    node->visit_children(this);
    if (node->basetype == bt_object){
        getClassInfo(node->objectClassName, this); // implied check for existence
    }
    for (IdentifierNode* id : *node->identifier_list){
        VariableInfo var;
        var.type = compoundFromTypeNode(node->type);
        //  typeid = b
        if (node->basetype == bt_object){
            var.size = classTable->at(var.type.objectClassName).membersSize; //check the size that this should be
        }
        else { var.size = 4; }
        var.offset = currentLocalOffset;
        if (currentLocalOffset < 0) {
            currentLocalOffset -= 4;
        } else {
            currentLocalOffset += 4;
        }

        (*(this->currentVariableTable))[id->name] = var;
    }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
    // WRITEME: Replace with code if necessary // WRITEME: Replace with code if necessary
    node->visit_children(this);
    //
    //
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
    node->visit_children(this);
    CompoundType expressionType = getExpressionType(node->expression, this);
    if (node->identifier_2 != NULL){ //NULL or empty string?
        //check classtable at name off class of 1st ID
        VariableInfo newVar = getVariableInfo(node->identifier_1->name, this);
        if (newVar.type.baseType != bt_object){
          typeError(not_object);
        }
        VariableInfo expressionVariableInfo= getVariableInfoFromClassMember(newVar.type.objectClassName, node->identifier_2->name, this);
        if (expressionVariableInfo.type.baseType == bt_none && expressionVariableInfo.type.objectClassName == "failure"){
          typeError(undefined_member);
        }
        if (!areSameCompoundType(expressionType, expressionVariableInfo.type)){
          typeError(assignment_type_mismatch);
        }
    }

  else if (node->identifier_2 == NULL){ //NULL or empty string?
    //check classtable at name of class of 1st ID
    VariableInfo assignedVar = getVariableInfo(node->identifier_1->name, this);
    if (assignedVar.type.baseType == bt_none && assignedVar.type.objectClassName == "failure"){
      typeError(undefined_member);
    }
    if (!areSameCompoundType(expressionType, assignedVar.type)){
      typeError(assignment_type_mismatch);
    }


  }

    node->visit_children(this);
    // check for errors: assignment_type_mismatch not_object undefined_member undefined_variable

}

void TypeCheck::visitCallNode(CallNode* node) {
    // WRITEME: Replace with code if necessary
    node->visit_children(this);
}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
    // WRITEME: Replace with code if necessary
    if (getExpressionType(node->expression, this).baseType != bt_boolean){
        typeError(if_predicate_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitWhileNode(WhileNode* node) {
    // WRITEME: Replace with code if necessary
    if (getExpressionType(node->expression, this).baseType != bt_boolean){
        typeError(while_predicate_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
    // WRITEME: Replace with code if necessary
    if (getExpressionType(node->expression, this).baseType != bt_boolean){
        typeError(while_predicate_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitPrintNode(PrintNode* node) {
    // WRITEME: Replace with code if necessary
    node->visit_children(this);
}






// expression nodes:
void TypeCheck::visitPlusNode(PlusNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitMinusNode(MinusNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitTimesNode(TimesNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitDivideNode(DivideNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitEqualNode(EqualNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_integer)
            || (getExpressionType(node->expression_2, this).baseType != bt_integer)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitAndNode(AndNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_boolean)
            || (getExpressionType(node->expression_2, this).baseType != bt_boolean)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitOrNode(OrNode* node) {
    // WRITEME: Replace with code if necessary
    if ((getExpressionType(node->expression_1, this).baseType != bt_boolean)
            || (getExpressionType(node->expression_2, this).baseType != bt_boolean)){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitNotNode(NotNode* node) {
    // WRITEME: Replace with code if necessary
    if (getExpressionType(node->expression, this).baseType != bt_boolean){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

void TypeCheck::visitNegationNode(NegationNode* node) {
    // WRITEME: Replace with code if necessary
    if (getExpressionType(node->expression, this).baseType != bt_integer){
        typeError(expression_type_mismatch);
    }
    node->visit_children(this);
}

// doing
void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
    // WRITEME: Replace with code if necessary
/*
    std::string classname = currentClassName;
    MethodInfo calledMethod;
    if (node->identifier_2 != NULL){
        VariableInfo var = getVariableInfo(node->identifier_1->name, this);
        if (var.type.baseType != bt_object){
            typeError(not_object);
        }
        classname = var.type.objectClassName;
        calledMethod = getMethodInfoFromClass(classname, node->identifier_2->name, this);
    }
    else {
        calledMethod = getMethodInfoFromClass(currentClassName, node->identifier_1->name, this);
    } */
    // next, check stuff about calledMethod

    // instead, I think we may be able to just do:
    getExpressionType(node, this);

    node->visit_children(this); //need to check expressions in params
}

// doing
void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
    // WRITEME: Replace with code if necessary

    VariableInfo var = getVariableInfo(node->identifier_1->name, this); //checks for undefined variable
    if (var.type.baseType != bt_object){
        typeError(not_object);
    }
    VariableInfo assignedType = getVariableInfoFromClassMember(var.type.objectClassName, node->identifier_2->name, this);
    if (assignedType.type.baseType == bt_none && assignedType.type.objectClassName == "failure"){
        typeError(undefined_member);
    }

    node->visit_children(this);

}

void TypeCheck::visitVariableNode(VariableNode* node) {
    // WRITEME: Replace with code if necessary
    getVariableInfo(node->identifier->name, this); //does undef_var check
    node->visit_children(this);
}

//nothing to do here?
void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    // WRITEME: Replace with code if necessary
}

//nothing to do here?
void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    // WRITEME: Replace with code if necessary
}

//done(?)
void TypeCheck::visitNewNode(NewNode* node) {
    // WRITEME: Replace with code if necessary
    getExpressionType(node, this);
    node->visit_children(this);
}

void TypeCheck::visitNoneNode(NoneNode* node) {
    // WRITEME: Replace with code if necessary
    node->basetype = bt_none;
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
    // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
    // WRITEME: Replace with code if necessary
}


// done?
void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
    // WRITEME: Replace with code if necessary
    // pretty sure nothing to do here
    node->basetype = bt_integer;
}


// done?
void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
    // WRITEME: Replace with code if necessary
    // pretty sure nothing to do here
    node->basetype = bt_boolean;
}

// done?
void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
    // WRITEME: Replace with code if necessary
    // There might be something to do there
    getClassInfo(node->identifier->name, this); // this should simply check that
                                //there is a class by that name to have as a type
    node->basetype = bt_object;
    node->objectClassName = node->identifier->name;
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
    std::string string = std::string("");
    for (int i = 0; i < indent; i++)
        string += std::string(" ");
    return string;
}

std::string string(CompoundType type) {
    switch (type.baseType) {
        case bt_integer:
        return std::string("Integer");
        case bt_boolean:
        return std::string("Boolean");
        case bt_none:
        return std::string("None");
        case bt_object:
        return std::string("Object(") + type.objectClassName + std::string(")");
        default:
        return std::string("");
    }
}


void print(VariableTable variableTable, int indent) {
    std::cout << genIndent(indent) << "VariableTable {";
    if (variableTable.size() == 0) {
        std::cout << "}";
        return;
    }
    std::cout << std::endl;
    for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
        std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
        std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
        if (it != --variableTable.end())
            std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
    std::cout << genIndent(indent) << "MethodTable {";
    if (methodTable.size() == 0) {
        std::cout << "}";
        return;
    }
    std::cout << std::endl;
    for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
        std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
        std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
        std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
        print(*it->second.variables, indent + 4);
        std::cout <<std::endl;
        std::cout << genIndent(indent + 2) << "}";
        if (it != --methodTable.end())
            std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
    std::cout << genIndent(indent) << "ClassTable {" << std::endl;
    for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
        std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
        if (it->second.superClassName != "")
            std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
        print(*it->second.members, indent + 4);
        std::cout << "," << std::endl;
        print(*it->second.methods, indent + 4);
        std::cout <<std::endl;
        std::cout << genIndent(indent + 2) << "}";
        if (it != --classTable.end())
            std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
    print(classTable, 0);
}
