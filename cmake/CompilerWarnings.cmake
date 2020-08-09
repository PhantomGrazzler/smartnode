# Basis taken from here:
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

function(set_project_warnings project_name)
  option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" TRUE)

  set(MSVC_WARNINGS
        /MP                     # Enable multi-processor compilation

        /experimental:external  # Enable external compiler options
        /external:anglebrackets # Treat all headers included via angle brackets as external headers
        /external:W0            # Turn off warnings for external headers

        /permissive-            # Enforce standards conformance

        /W4     # Enable warning level 4
        /w14242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254 # 'operator': conversion from 'type1' to 'type2', possible loss of data
        /w14263 # 'function': member function does not override any base class virtual member function
        /w14265 # 'class': class has virtual functions, but destructor is not virtual
        /w14287 # 'operator': unsigned/negative constant mismatch
        /w14289 # nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
        /w14296 # 'operator': expression is always false
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side-effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # 'instance': construction of local static object is not thread-safe
        /w14826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
  )

  set(CLANG_WARNINGS
        -Wall                   # Good subset of warnings enabled
        -Wextra                 # Additional good subset of warnings enabled
        -Wpedantic              # Warn if non-standard C++ is used
        -Wshadow                # warn the user if a variable declaration shadows one from a parent context
        -Wold-style-cast        # warn for c-style casts
        -Wcast-align            # warn for potential performance problem casts
        -Wunused                # warn on anything being unused
        -Woverloaded-virtual    # warn if you overload (not override) a virtual function
        -Wconversion            # warn on type conversions that may lose data
        -Wsign-conversion       # warn on sign conversions
        -Wdouble-promotion      # (GCC >= 4.6, Clang >= 3.8) warn if float is implicit promoted to double
        -Wformat=2              # warn on security issues around functions that format output (ie printf)
        -Wnull-dereference      # warn if a null dereference is detected
  )

  if (WARNINGS_AS_ERRORS)
    set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
    set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
  endif()

  set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wduplicated-cond       # (only in GCC >= 6.0) warn if if / else chain has duplicated conditions
        -Wduplicated-branches   # (only in GCC >= 7.0) warn if if / else branches have duplicated code
        -Wlogical-op            # (only in GCC) warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast          # (only in GCC >= 4.8) warn if you perform a cast to the same type
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
  )

  if(MSVC)
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  else()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
  endif()

  target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})
endfunction()
