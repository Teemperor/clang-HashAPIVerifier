// Declares clang::SyntaxOnlyAction.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/RecursiveASTVisitor.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory MyToolCategory("clang-clone-finder options");

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

bool hasEnding (std::string const &fullString, std::string const &ending) {
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
  std::regex("class clang::SourceLocation"),
  std::regex("[\\s\\S]*_begin[\\s\\S]*"),
  std::regex("[\\s\\S]*_end[\\s\\S]*"),
  std::regex("[\\s\\S]*::begin_[^:]+"),
  std::regex("[\\s\\S]*::end_[^:]+")
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

int main(int argc, const char **argv) {
  assert(argc == 3);

  std::string Code = getFileContents(argv[1]);

  std::vector<std::string> Args = {"-D__STDC_LIMIT_MACROS",
                                   "-D__STDC_CONSTANT_MACROS",
                                   "-std=c++11", "-I/usr/lib/clang/3.8.0/include/",
                                   "-I/data/llvm/gsoc2016/include",
                                   "-I/data/llvm/gsoc2016/tools/clang/include",
                                   "-DCLANG_ENABLE_ARCMT", "-DCLANG_ENABLE_OBJC_REWRITER",
                                   "-DCLANG_ENABLE_STATIC_ANALYZER", "-DGTEST_HAS_RTTI=0",
                                   "-D_DEBUG", "-D_GNU_SOURCE", "-D__STDC_CONSTANT_MACROS",
                                   "-D__STDC_FORMAT_MACROS", "-D__STDC_LIMIT_MACROS",
                                   "-I/data/llvm/gsoc2016-build/tools/clang/lib/AST",
                                   "-I/data/llvm/gsoc2016/tools/clang/lib/AST",
                                   "-I/data/llvm/gsoc2016/tools/clang/include",
                                   "-I/data/llvm/gsoc2016-build/tools/clang/include",
                                   "-I/data/llvm/gsoc2016-build/include",
                                   "-I/data/llvm/gsoc2016/include", "-fPIC",
                                   "-fvisibility-inlines-hidden",
                                   "-std=c++11",
                                   "-ffunction-sections", "-fdata-sections",
                                   "-fno-common", "-Woverloaded-virtual", "-fno-strict-aliasing",
                                   "-O3", "-UNDEBUG", "-fno-exceptions", "-fno-rtti"};
  auto AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args, "ASTStructure.cpp");

  FindVisitMethodsVisitor Visitor;
  Visitor.TraverseTranslationUnitDecl(AST->getASTContext().getTranslationUnitDecl());

  std::set<std::string> UnusedList = ReadLines(argv[2]);

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

  if (NotFoundCalls.empty()) {
      return 0;
  } else {
    std::cerr << "Following methods are never called and not marked unused:\n";
    for (auto& Call : NotFoundCalls) {
      std::cerr << Call << "\n";
    }
    return 1;
  }
}
