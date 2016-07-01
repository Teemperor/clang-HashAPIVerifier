# HashAPIVerifier

Checks if the code in ASTStructure.cpp is using all members in the clang AST API for statements.

`HashApiVerifier PATH-TO-IGNORE-LIST COMP-DB-PATH`

where PATH-TO-IGNORE-LIST is a path to a list of qualified function names
that aren't used in ASTStructure.cpp but should not be reported.

Example content of this file looks like this:

    # Comment
    clang::Expr::refersToVectorElement
    clang::Expr::skipRValueSubobjectAdjustments
    clang::Expr::tryEvaluateObjectSize
    clang::ExprWithCleanups::getNumObjects
    clang::ExprWithCleanups::getObject
    clang::ExprWithCleanups::getObjects
    clang::ExprWithCleanups::getSubExpr
    clang::ExpressionTraitExpr::getQueriedExpression
    clang::ExpressionTraitExpr::getValue

COMP-DB-PATH is a path to the **directory** containing a compilation
database for clang. This is needed to retrieve the compiler arguments
necessary to parse ASTStructure.cpp and all relevant headers.

This program ensures that ASTStructure will always correctly
use the clang AST API methods and should detect if the
API was extended with methods.

It doesn't check if new classes are added to the clang AST API,
because ASTStructure.cpp will only compile if visit methods
for all classes are implemented.

