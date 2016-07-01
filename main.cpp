// Declares clang::SyntaxOnlyAction.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <cstdlib>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/CommandLine.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

std::set<std::string> UnusedList;

std::string getFileContents(const std::string& path) {
  std::ifstream inputStream(path);
  std::stringstream buffer;
  buffer << inputStream.rdbuf();
  return buffer.str();
}

std::set<std::string> ReadLines(const std::string& path) {
  std::set<std::string> Result;

  std::ifstream myfile (path);
  std::string Line;
  if (myfile.is_open()) {
    while (getline(myfile, Line)) {
      if (!Line.empty() && !(Line[0] == '#'))
        Result.insert(Line);
    }
    myfile.close();
  } else {
    assert(false || "Can't open file");
  }

  return Result;
}

bool EndsWith(std::string const &fullString, std::string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

class ListMethodVisitor
        : public RecursiveASTVisitor<ListMethodVisitor> {

  const CXXRecordDecl *OnlyForDecl;

public:
  ListMethodVisitor(const CXXRecordDecl *OnlyForDecl,
                    std::set<std::string>& MemberCalls)
    : OnlyForDecl(OnlyForDecl), MemberCalls(MemberCalls) {

  }

  std::set<std::string>& MemberCalls;

  bool VisitCXXMemberCallExpr(CXXMemberCallExpr *E) {
    if (E->getRecordDecl() == OnlyForDecl) {
      MemberCalls.insert(E->getMethodDecl()->getQualifiedNameAsString());
    }
    return true;
  }
};

class FindVisitMethodsVisitor
        : public RecursiveASTVisitor<FindVisitMethodsVisitor> {

public:
  std::set<std::string> MemberCalls;
  std::set<const CXXRecordDecl *> APIClasses;

  bool VisitFunctionDecl(FunctionDecl *D) {

    if (D->getQualifiedNameAsString().find("StructuralHashVisitor::Visit") != std::string::npos) {

      const CXXRecordDecl* ClangAPIClass = nullptr;
      if (D->param_size() == 1) {
        QualType ArgType = (*D->param_begin())->getOriginalType();
        ClangAPIClass = ArgType->getPointeeCXXRecordDecl();
        if (ClangAPIClass) {
          APIClasses.insert(ClangAPIClass);
        }
      }
      ListMethodVisitor Visitor(ClangAPIClass, MemberCalls);
      Visitor.TraverseStmt(D->getBody());
    }
    return true;
  }
};

const std::vector<std::regex> ReturnTypeFilters = {
  std::regex("class clang::SourceLocation")
};

const std::vector<std::regex> NameFilters = {
  std::regex("[\\s\\S]*_begin"),
  std::regex("[\\s\\S]*_end"),
  std::regex("[\\s\\S]*::begin_[^:]+"),
  std::regex("[\\s\\S]*::end_[^:]+")
};

bool MatchesAnyFilter(const std::string& Input, const std::vector<std::regex>& Filters) {
  for (const std::regex& Filter : Filters) {
    if (std::regex_match(Input, Filter)) {
      return true;
    }
  }
  return false;
}

bool ShouldCheckMethod(const CXXMethodDecl *D) {
  QualType ReturnType = D->getReturnType();
  return
      D->isInstance() &&
      D->getAccess() == AccessSpecifier::AS_public && D->isConst() &&
      !MatchesAnyFilter(ReturnType.getAsString(), ReturnTypeFilters) &&
      !MatchesAnyFilter(D->getQualifiedNameAsString(), NameFilters);

}

class VerifyHashingCodeConsumer : public clang::ASTConsumer {
public:
  explicit VerifyHashingCodeConsumer (ASTContext *Context)
    : Context(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    FindVisitMethodsVisitor Visitor;
    Visitor.TraverseTranslationUnitDecl(Context.getTranslationUnitDecl());

    std::set<std::string> MemberCalls = Visitor.MemberCalls;

    std::set<std::string> AllMemberCalls;

    std::vector<std::string> NotFoundCalls;

    for (const CXXRecordDecl *CD : Visitor.APIClasses) {
      for (const CXXMethodDecl *MD : CD->methods()) {
        if (ShouldCheckMethod(MD)) {
          AllMemberCalls.insert(MD->getQualifiedNameAsString());
        }
      }
    }

    for (auto& Call : AllMemberCalls) {
      if (UnusedList.find(Call) == UnusedList.end() &&
          MemberCalls.find(Call) == MemberCalls.end()) {
        NotFoundCalls.push_back(Call);
      }
    }

    if (!NotFoundCalls.empty()) {
      std::cout << "Following methods are never called and not marked unused:\n";
      for (auto& Call : NotFoundCalls) {
        std::cout << Call << "\n";
      }
      std::exit(1);
    }
  }
private:
  ASTContext *Context;
};

class VerifyHashingCodeAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new VerifyHashingCodeConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, const char **argv) {
  if (argc != 3) {
    std::cerr << "You need to invoke this program like this:\n";
    std::cerr << argv[0] << " PATH-TO-IGNORE-LIST COMP-DB-PATH\n";
  }
  UnusedList = ReadLines(argv[1])

  std::string Error;
  auto DB = CompilationDatabase::loadFromDirectory(argv[2], Error);
  if (!Error.empty()) {
    std::cerr << Error << std::endl;
    assert(false && "Error while reading compilation database");
  }

  for (const CompileCommand& Command : DB->getAllCompileCommands()) {
    CompileCommand C = Command;
    if (EndsWith(C.Filename, "ASTStructure.cpp")) {
      std::vector<std::string> Args = C.CommandLine;

      FileManager* Manager = new FileManager(FileSystemOptions());
      clang::tooling::ToolInvocation Invocation(Args, new VerifyHashingCodeAction(), Manager);
      Invocation.run();
      return 0;
    }
  }
  std::cerr << "Can't find compilation command for ASTStructure.cpp in DB.";
}
