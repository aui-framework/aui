Checks: >
  bugprone-*,
  clang-diagnostic-*,
  clang-analyzer-*,
  cppcoreguidelines-*,
  modernize-*,
  -modernize-use-trailing-return-type,
  -clang-analyzer-optin.core.EnumCastOutOfRange,
  -bugprone-assignment-in-if-condition,
  -bugprone-branch-clone,
  -bugprone-casting-through-void,
  -bugprone-easily-swappable-parameters,
  -bugprone-empty-catch,
  -bugprone-exception-escape,
  -bugprone-forward-declaration-namespace,
  -bugprone-forwarding-reference-overload,
  -bugprone-implicit-widening-of-multiplication-result,
  -bugprone-infinite-loop,
  -bugprone-macro-parentheses,
  -bugprone-move-forwarding-reference,
  -bugprone-multi-level-implicit-pointer-conversion,
  -bugprone-narrowing-conversions,
  -bugprone-parent-virtual-call,
  -bugprone-reserved-identifier,
  -bugprone-sizeof-expression,
  -bugprone-string-literal-with-embedded-nul,
  -bugprone-switch-missing-default-case,
  -bugprone-too-small-loop-variable,
  -bugprone-use-after-move,
  -clang-diagnostic-braced-scalar-init,
  -clang-diagnostic-deprecated-declarations,
  -clang-diagnostic-deprecated-this-capture,
  -clang-diagnostic-inconsistent-missing-override,
  -clang-diagnostic-null-dereference,
  -clang-diagnostic-potentially-evaluated-expression,
  -clang-diagnostic-switch,
  -clang-diagnostic-unused-value,
  -cppcoreguidelines-avoid-c-arrays,
  -cppcoreguidelines-avoid-const-or-ref-data-members,
  -cppcoreguidelines-avoid-do-while,
  -cppcoreguidelines-avoid-goto,
  -cppcoreguidelines-avoid-magic-numbers,
  -cppcoreguidelines-avoid-non-const-global-variables,
  -cppcoreguidelines-c-copy-assignment-signature,
  -cppcoreguidelines-explicit-virtual-functions,
  -cppcoreguidelines-init-variables,
  -cppcoreguidelines-macro-usage,
  -cppcoreguidelines-misleading-capture-default-by-value,
  -cppcoreguidelines-missing-std-forward,
  -cppcoreguidelines-narrowing-conversions,
  -cppcoreguidelines-no-malloc,
  -cppcoreguidelines-noexcept-move-operations,
  -cppcoreguidelines-non-private-member-variables-in-classes,
  -cppcoreguidelines-owning-memory,
  -cppcoreguidelines-prefer-member-initializer,
  -cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  -cppcoreguidelines-pro-bounds-constant-array-index,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -cppcoreguidelines-pro-type-const-cast,
  -cppcoreguidelines-pro-type-cstyle-cast,
  -cppcoreguidelines-pro-type-member-init,
  -cppcoreguidelines-pro-type-reinterpret-cast,
  -cppcoreguidelines-pro-type-union-access,
  -cppcoreguidelines-pro-type-vararg,
  -cppcoreguidelines-rvalue-reference-param-not-moved,
  -cppcoreguidelines-special-member-functions,
  -cppcoreguidelines-virtual-class-destructor,
  -modernize-avoid-c-arrays,
  -modernize-concat-nested-namespaces,
  -modernize-deprecated-headers,
  -modernize-loop-convert,
  -modernize-pass-by-value,
  -modernize-raw-string-literal,
  -modernize-return-braced-init-list,
  -modernize-type-traits,
  -modernize-use-auto,
  -modernize-use-constraints,
  -modernize-use-default-member-init,
  -modernize-use-bool-literals,
  -modernize-use-equals-default,
  -modernize-use-equals-delete,
  -modernize-use-nodiscard,
  -modernize-use-nullptr,
  -modernize-use-override,
  -modernize-use-using,


# we have a pipeline that treats those as errors; during development, it may be too noisy
#WarningsAsErrors: '*'


HeaderFilterRegex: '^.*/src/AUI/.*$'
FormatStyle:     google
CheckOptions:
  - key:             cert-dcl16-c.NewSuffixes
    value:           'L;LL;LU;LLU'
  - key:             cert-oop54-cpp.WarnOnlyIfThisHasSuspiciousField
    value:           '0'
  - key:             cppcoreguidelines-explicit-virtual-functions.IgnoreDestructors
    value:           '1'
  - key:             cppcoreguidelines-non-private-member-variables-in-classes.IgnoreClassesWithAllMemberVariablesBeingPublic
    value:           '1'
  - key:             google-readability-braces-around-statements.ShortStatementLines
    value:           '1'
  - key:             google-readability-function-size.StatementThreshold
    value:           '800'
  - key:             google-readability-namespace-comments.ShortNamespaceLines
    value:           '10'
  - key:             google-readability-namespace-comments.SpacesBeforeComments
    value:           '2'
  - key:             modernize-loop-convert.MaxCopySize
    value:           '16'
  - key:             modernize-loop-convert.MinConfidence
    value:           reasonable
  - key:             modernize-loop-convert.NamingStyle
    value:           CamelCase
  - key:             modernize-pass-by-value.IncludeStyle
    value:           llvm
  - key:             modernize-replace-auto-ptr.IncludeStyle
    value:           llvm
  - key:             modernize-use-nullptr.NullMacros
    value:           'NULL'