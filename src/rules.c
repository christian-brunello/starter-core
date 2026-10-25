/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 20 "rules.y"

/* C Decls */
#include <stdio.h>
#include <glib.h>

#include <starter/defs.h>

#include "internals.h"

extern int yylineno;
extern int yylex (void);
void yyerror (Engine *, char const *);


#line 86 "rules.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_RULES_H_INCLUDED
# define YY_YY_RULES_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_EQ = 258,                    /* T_EQ  */
    T_NE = 259,                    /* T_NE  */
    T_LT = 260,                    /* T_LT  */
    T_LE = 261,                    /* T_LE  */
    T_GT = 262,                    /* T_GT  */
    T_GE = 263,                    /* T_GE  */
    T_AND = 264,                   /* T_AND  */
    T_OR = 265,                    /* T_OR  */
    T_KEY_VAR = 266,               /* T_KEY_VAR  */
    T_KEY_SET = 267,               /* T_KEY_SET  */
    T_KEY_WHEN = 268,              /* T_KEY_WHEN  */
    T_KEY_TRIGGER = 269,           /* T_KEY_TRIGGER  */
    T_KEY_FLAG = 270,              /* T_KEY_FLAG  */
    VALUE = 271,                   /* VALUE  */
    IDENTIFIER = 272,              /* IDENTIFIER  */
    T_FLAG_NAME = 273,             /* T_FLAG_NAME  */
    UMINUS = 274                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define T_EQ 258
#define T_NE 259
#define T_LT 260
#define T_LE 261
#define T_GT 262
#define T_GE 263
#define T_AND 264
#define T_OR 265
#define T_KEY_VAR 266
#define T_KEY_SET 267
#define T_KEY_WHEN 268
#define T_KEY_TRIGGER 269
#define T_KEY_FLAG 270
#define VALUE 271
#define IDENTIFIER 272
#define T_FLAG_NAME 273
#define UMINUS 274

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 40 "rules.y"

    double dval; 
    gchar *sval; 
    guint64 uval;

    Expr *expr_ptr;
    Assign *ass_ptr;
    Trigger *trigger_ptr;
    LabelEntry *label_entry_ptr;
    LabelBlock *label_block_ptr;
    GPtrArray *ptr_array; 

#line 190 "rules.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (Engine *engine);


#endif /* !YY_YY_RULES_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_T_EQ = 3,                       /* T_EQ  */
  YYSYMBOL_T_NE = 4,                       /* T_NE  */
  YYSYMBOL_T_LT = 5,                       /* T_LT  */
  YYSYMBOL_T_LE = 6,                       /* T_LE  */
  YYSYMBOL_T_GT = 7,                       /* T_GT  */
  YYSYMBOL_T_GE = 8,                       /* T_GE  */
  YYSYMBOL_T_AND = 9,                      /* T_AND  */
  YYSYMBOL_T_OR = 10,                      /* T_OR  */
  YYSYMBOL_T_KEY_VAR = 11,                 /* T_KEY_VAR  */
  YYSYMBOL_T_KEY_SET = 12,                 /* T_KEY_SET  */
  YYSYMBOL_T_KEY_WHEN = 13,                /* T_KEY_WHEN  */
  YYSYMBOL_T_KEY_TRIGGER = 14,             /* T_KEY_TRIGGER  */
  YYSYMBOL_T_KEY_FLAG = 15,                /* T_KEY_FLAG  */
  YYSYMBOL_VALUE = 16,                     /* VALUE  */
  YYSYMBOL_IDENTIFIER = 17,                /* IDENTIFIER  */
  YYSYMBOL_T_FLAG_NAME = 18,               /* T_FLAG_NAME  */
  YYSYMBOL_19_ = 19,                       /* '+'  */
  YYSYMBOL_20_ = 20,                       /* '-'  */
  YYSYMBOL_21_ = 21,                       /* '*'  */
  YYSYMBOL_22_ = 22,                       /* '/'  */
  YYSYMBOL_UMINUS = 23,                    /* UMINUS  */
  YYSYMBOL_24_ = 24,                       /* '{'  */
  YYSYMBOL_25_ = 25,                       /* '}'  */
  YYSYMBOL_26_ = 26,                       /* ';'  */
  YYSYMBOL_27_ = 27,                       /* ','  */
  YYSYMBOL_28_ = 28,                       /* '='  */
  YYSYMBOL_29_ = 29,                       /* '('  */
  YYSYMBOL_30_ = 30,                       /* ')'  */
  YYSYMBOL_YYACCEPT = 31,                  /* $accept  */
  YYSYMBOL_program = 32,                   /* program  */
  YYSYMBOL_instruction_list = 33,          /* instruction_list  */
  YYSYMBOL_instruction = 34,               /* instruction  */
  YYSYMBOL_label_block = 35,               /* label_block  */
  YYSYMBOL_label_entries = 36,             /* label_entries  */
  YYSYMBOL_label_entry = 37,               /* label_entry  */
  YYSYMBOL_trigger_statement = 38,         /* trigger_statement  */
  YYSYMBOL_assignment_list = 39,           /* assignment_list  */
  YYSYMBOL_assignment = 40,                /* assignment  */
  YYSYMBOL_declaration = 41,               /* declaration  */
  YYSYMBOL_expr = 42,                      /* expr  */
  YYSYMBOL_argument_list = 43              /* argument_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  14
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   194

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  31
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  13
/* YYNRULES -- Number of rules.  */
#define YYNRULES  40
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  86

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   274


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      29,    30,    21,    19,    27,    20,     2,    22,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    26,
       2,    28,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    24,     2,    25,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    23
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    84,    84,    88,    89,    93,    97,   101,   105,   122,
     144,   148,   154,   162,   170,   175,   182,   188,   197,   203,
     210,   213,   217,   223,   228,   233,   238,   243,   248,   253,
     258,   264,   269,   274,   279,   284,   287,   293,   353,   354,
     358
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "T_EQ", "T_NE", "T_LT",
  "T_LE", "T_GT", "T_GE", "T_AND", "T_OR", "T_KEY_VAR", "T_KEY_SET",
  "T_KEY_WHEN", "T_KEY_TRIGGER", "T_KEY_FLAG", "VALUE", "IDENTIFIER",
  "T_FLAG_NAME", "'+'", "'-'", "'*'", "'/'", "UMINUS", "'{'", "'}'", "';'",
  "','", "'='", "'('", "')'", "$accept", "program", "instruction_list",
  "instruction", "label_block", "label_entries", "label_entry",
  "trigger_statement", "assignment_list", "assignment", "declaration",
  "expr", "argument_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-25)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      57,   -11,    -3,   -13,    46,    57,   -25,   -25,   -25,   -25,
      44,    23,    33,    61,   -25,   -25,   -25,    23,   -25,    -6,
      23,    23,   104,    45,    23,    16,   -25,    56,    62,    23,
     -25,    28,    23,    23,    23,    23,    23,    23,    23,    23,
      64,    23,    23,    23,    23,    61,   124,   -25,   -25,   -25,
     -25,   144,   -22,   -25,   162,   162,    72,    72,    72,    72,
     152,   172,    14,    29,   -25,    -8,    -8,   -25,   -25,    18,
      23,    23,   -25,    77,    23,   -25,    64,   -25,    80,   144,
      51,   144,   -25,   -25,    23,   144
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     2,     3,     5,     6,     7,
       0,     0,     0,     0,     1,     4,    18,     0,    20,    21,
       0,     0,     0,     0,     0,     0,    10,     0,     0,    38,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,    11,    19,
      22,    39,     0,    35,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,    14,    31,    32,    33,    34,     0,
       0,     0,    37,     0,     0,    13,     0,     9,     0,    40,
       0,    16,    15,    12,     0,    17
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -25,   -25,   -25,    91,   -25,    52,   -24,   -25,   -25,    22,
     -25,   -17,   -25
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     4,     5,     6,     7,    25,    26,     8,    63,    64,
       9,    22,    52
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      27,    48,    12,    30,    31,    71,    10,    46,    72,    28,
      11,    13,    51,    43,    44,    54,    55,    56,    57,    58,
      59,    60,    61,    29,    65,    66,    67,    68,    24,    73,
      24,    32,    33,    34,    35,    36,    37,    38,    39,    18,
      19,    47,    74,    77,    20,    48,    14,    41,    42,    43,
      44,    23,    21,    78,    79,    75,    76,    81,    53,    32,
      33,    34,    35,    36,    37,    38,    39,    85,     1,    45,
      16,     2,    17,    24,     3,    41,    42,    43,    44,    84,
      50,    62,    49,    32,    33,    34,    35,    36,    37,    38,
      39,    41,    42,    43,    44,    80,    15,    69,    82,    41,
      42,    43,    44,     0,     0,     0,    83,    32,    33,    34,
      35,    36,    37,    38,    39,     0,    40,     0,     0,     0,
       0,     0,     0,    41,    42,    43,    44,    32,    33,    34,
      35,    36,    37,    38,    39,     0,     0,    70,     0,     0,
       0,     0,     0,    41,    42,    43,    44,    32,    33,    34,
      35,    36,    37,    38,    39,    32,    33,    34,    35,    36,
      37,     0,    39,    41,    42,    43,    44,    34,    35,    36,
      37,    41,    42,    43,    44,    32,    33,    34,    35,    36,
      37,    41,    42,    43,    44,     0,     0,     0,     0,     0,
       0,    41,    42,    43,    44
};

static const yytype_int8 yycheck[] =
{
      17,    25,    15,    20,    21,    27,    17,    24,    30,    15,
      13,    24,    29,    21,    22,    32,    33,    34,    35,    36,
      37,    38,    39,    29,    41,    42,    43,    44,    12,    15,
      12,     3,     4,     5,     6,     7,     8,     9,    10,    16,
      17,    25,    28,    25,    21,    69,     0,    19,    20,    21,
      22,    18,    29,    70,    71,    26,    27,    74,    30,     3,
       4,     5,     6,     7,     8,     9,    10,    84,    11,    24,
      26,    14,    28,    12,    17,    19,    20,    21,    22,    28,
      18,    17,    26,     3,     4,     5,     6,     7,     8,     9,
      10,    19,    20,    21,    22,    18,     5,    45,    76,    19,
      20,    21,    22,    -1,    -1,    -1,    26,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,     3,     4,     5,
       6,     7,     8,     9,    10,     3,     4,     5,     6,     7,
       8,    -1,    10,    19,    20,    21,    22,     5,     6,     7,
       8,    19,    20,    21,    22,     3,     4,     5,     6,     7,
       8,    19,    20,    21,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,    21,    22
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    11,    14,    17,    32,    33,    34,    35,    38,    41,
      17,    13,    15,    24,     0,    34,    26,    28,    16,    17,
      21,    29,    42,    18,    12,    36,    37,    42,    15,    29,
      42,    42,     3,     4,     5,     6,     7,     8,     9,    10,
      12,    19,    20,    21,    22,    24,    42,    25,    37,    26,
      18,    42,    43,    30,    42,    42,    42,    42,    42,    42,
      42,    42,    17,    39,    40,    42,    42,    42,    42,    36,
      13,    27,    30,    15,    28,    26,    27,    25,    42,    42,
      18,    42,    40,    26,    28,    42
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    31,    32,    33,    33,    34,    34,    34,    35,    35,
      36,    36,    37,    38,    39,    39,    40,    40,    41,    41,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    43,    43,
      43
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     4,     6,
       1,     2,     5,     6,     1,     3,     3,     5,     3,     5,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     4,     0,     1,
       3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (engine, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, engine); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, Engine *engine)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (engine);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, Engine *engine)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, engine);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, Engine *engine)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], engine);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, engine); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, Engine *engine)
{
  YY_USE (yyvaluep);
  YY_USE (engine);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (Engine *engine)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 5: /* instruction: label_block  */
#line 94 "rules.y"
    {
      g_ptr_array_add(engine->labels, (yyvsp[0].label_block_ptr));
    }
#line 1556 "rules.c"
    break;

  case 6: /* instruction: trigger_statement  */
#line 98 "rules.y"
    {
      g_ptr_array_add(engine->triggers, (yyvsp[0].trigger_ptr));
    }
#line 1564 "rules.c"
    break;

  case 7: /* instruction: declaration  */
#line 101 "rules.y"
                  { }
#line 1570 "rules.c"
    break;

  case 8: /* label_block: IDENTIFIER '{' label_entries '}'  */
#line 106 "rules.y"
    {
      guint i;

      for(i = 0; i < (yyvsp[-1].ptr_array)->len; i++)
        {
	  LabelEntry *e = (yyvsp[-1].ptr_array)->pdata[i];

	  g_free(e->ass->id);
	  e->ass->id = g_strdup((yyvsp[-3].sval));
	  e->ass->flag_bit = 0;
        }

      (yyval.label_block_ptr) = label_block_new((yyvsp[-1].ptr_array));

      free((yyvsp[-3].sval));
    }
#line 1591 "rules.c"
    break;

  case 9: /* label_block: IDENTIFIER T_KEY_FLAG T_FLAG_NAME '{' label_entries '}'  */
#line 123 "rules.y"
    {
      guint i;

      /* Non-USR write targets are accepted at parse time; assign_apply warns
       * and skips. */
      for(i = 0; i < (yyvsp[-1].ptr_array)->len; i++)
        {
	  LabelEntry *e = (yyvsp[-1].ptr_array)->pdata[i];

	  g_free(e->ass->id);
	  e->ass->id = g_strdup((yyvsp[-5].sval));
	  e->ass->flag_bit = (yyvsp[-3].uval);
        }

      (yyval.label_block_ptr) = label_block_new((yyvsp[-1].ptr_array));

      free((yyvsp[-5].sval));
    }
#line 1614 "rules.c"
    break;

  case 10: /* label_entries: label_entry  */
#line 144 "rules.y"
                {
      (yyval.ptr_array) = g_ptr_array_new_full(0, (GDestroyNotify) label_entry_delete);
      g_ptr_array_add((yyval.ptr_array), (yyvsp[0].label_entry_ptr));
    }
#line 1623 "rules.c"
    break;

  case 11: /* label_entries: label_entries label_entry  */
#line 148 "rules.y"
                                {
      g_ptr_array_add((yyvsp[-1].ptr_array), (yyvsp[0].label_entry_ptr));
    }
#line 1631 "rules.c"
    break;

  case 12: /* label_entry: T_KEY_SET expr T_KEY_WHEN expr ';'  */
#line 155 "rules.y"
    {
      (yyval.label_entry_ptr) = label_entry_new((yyvsp[-1].expr_ptr), assign_new("", (yyvsp[-3].expr_ptr)));
      expr_unref((yyvsp[-3].expr_ptr));
      expr_unref((yyvsp[-1].expr_ptr));
    }
#line 1641 "rules.c"
    break;

  case 13: /* trigger_statement: T_KEY_TRIGGER T_KEY_WHEN expr T_KEY_SET assignment_list ';'  */
#line 163 "rules.y"
                 {
		   (yyval.trigger_ptr) = trigger_new((yyvsp[-3].expr_ptr), (yyvsp[-1].ptr_array));
		   expr_unref((yyvsp[-3].expr_ptr));
		 }
#line 1650 "rules.c"
    break;

  case 14: /* assignment_list: assignment  */
#line 171 "rules.y"
    {
      (yyval.ptr_array) = g_ptr_array_new_full(0, (GDestroyNotify) assign_delete);
      g_ptr_array_add((yyval.ptr_array), (yyvsp[0].ass_ptr));
    }
#line 1659 "rules.c"
    break;

  case 15: /* assignment_list: assignment_list ',' assignment  */
#line 176 "rules.y"
    {
      g_ptr_array_add((yyval.ptr_array), (yyvsp[0].ass_ptr));  
    }
#line 1667 "rules.c"
    break;

  case 16: /* assignment: IDENTIFIER '=' expr  */
#line 183 "rules.y"
          {
	    (yyval.ass_ptr) = assign_new((yyvsp[-2].sval), (yyvsp[0].expr_ptr));
	    expr_unref((yyvsp[0].expr_ptr));
	    free((yyvsp[-2].sval));
	  }
#line 1677 "rules.c"
    break;

  case 17: /* assignment: IDENTIFIER T_KEY_FLAG T_FLAG_NAME '=' expr  */
#line 189 "rules.y"
          {
	    /* Non-USR: parse OK; assign_apply warns and skips. */
	    (yyval.ass_ptr) = assign_new_flag ((yyvsp[-4].sval), (yyvsp[-2].uval), (yyvsp[0].expr_ptr));
	    expr_unref ((yyvsp[0].expr_ptr));
	    free ((yyvsp[-4].sval));
	  }
#line 1688 "rules.c"
    break;

  case 18: /* declaration: T_KEY_VAR IDENTIFIER ';'  */
#line 197 "rules.y"
                             {
      Expr *v = expr_new_literal(0);
      g_hash_table_insert(engine->variables, (yyvsp[-1].sval), st_var_new((yyvsp[-1].sval), v));
      expr_unref(v);

    }
#line 1699 "rules.c"
    break;

  case 19: /* declaration: T_KEY_VAR IDENTIFIER '=' expr ';'  */
#line 203 "rules.y"
                                        {
      g_hash_table_insert(engine->variables, (yyvsp[-3].sval), st_var_new((yyvsp[-3].sval), (yyvsp[-1].expr_ptr)));
      expr_unref((yyvsp[-1].expr_ptr));
    }
#line 1708 "rules.c"
    break;

  case 20: /* expr: VALUE  */
#line 210 "rules.y"
            {
	(yyval.expr_ptr) = expr_new_literal((yyvsp[0].dval));
      }
#line 1716 "rules.c"
    break;

  case 21: /* expr: IDENTIFIER  */
#line 213 "rules.y"
                   {
	(yyval.expr_ptr) = expr_new_ref((yyvsp[0].sval));
	free((yyvsp[0].sval));
      }
#line 1725 "rules.c"
    break;

  case 22: /* expr: IDENTIFIER T_KEY_FLAG T_FLAG_NAME  */
#line 217 "rules.y"
                                          {
	/* Read: any known flag name (USR* / MANUAL_OVERRIDE / NO_HISTORY). */
	(yyval.expr_ptr) = expr_new_flag ((yyvsp[-2].sval), (yyvsp[0].uval));
	free ((yyvsp[-2].sval));
      }
#line 1735 "rules.c"
    break;

  case 23: /* expr: expr T_EQ expr  */
#line 223 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op("==", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1745 "rules.c"
    break;

  case 24: /* expr: expr T_NE expr  */
#line 228 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op("!=", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1755 "rules.c"
    break;

  case 25: /* expr: expr T_LT expr  */
#line 233 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op("<", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1765 "rules.c"
    break;

  case 26: /* expr: expr T_LE expr  */
#line 238 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op("<=", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1775 "rules.c"
    break;

  case 27: /* expr: expr T_GT expr  */
#line 243 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op(">", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1785 "rules.c"
    break;

  case 28: /* expr: expr T_GE expr  */
#line 248 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op(">=", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1795 "rules.c"
    break;

  case 29: /* expr: expr T_AND expr  */
#line 253 "rules.y"
                        {
	(yyval.expr_ptr) = expr_new_op("&&", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1805 "rules.c"
    break;

  case 30: /* expr: expr T_OR expr  */
#line 258 "rules.y"
                       {
	(yyval.expr_ptr) = expr_new_op("||", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1815 "rules.c"
    break;

  case 31: /* expr: expr '+' expr  */
#line 264 "rules.y"
                      {
	(yyval.expr_ptr) = expr_new_op("+", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1825 "rules.c"
    break;

  case 32: /* expr: expr '-' expr  */
#line 269 "rules.y"
                      {
	(yyval.expr_ptr) = expr_new_op("-", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1835 "rules.c"
    break;

  case 33: /* expr: expr '*' expr  */
#line 274 "rules.y"
                      {
	(yyval.expr_ptr) = expr_new_op("*", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1845 "rules.c"
    break;

  case 34: /* expr: expr '/' expr  */
#line 279 "rules.y"
                      {
	(yyval.expr_ptr) = expr_new_op("/", (yyvsp[-2].expr_ptr), (yyvsp[0].expr_ptr));
	expr_unref((yyvsp[-2].expr_ptr));
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1855 "rules.c"
    break;

  case 35: /* expr: '(' expr ')'  */
#line 284 "rules.y"
                     {
	(yyval.expr_ptr) = (yyvsp[-1].expr_ptr);
      }
#line 1863 "rules.c"
    break;

  case 36: /* expr: '*' expr  */
#line 287 "rules.y"
                              {
        Expr *le = expr_new_literal(-1);
	(yyval.expr_ptr) = expr_new_op("*", le, (yyvsp[0].expr_ptr));
	expr_unref(le);
	expr_unref((yyvsp[0].expr_ptr));
      }
#line 1874 "rules.c"
    break;

  case 37: /* expr: IDENTIFIER '(' argument_list ')'  */
#line 293 "rules.y"
                                         {
        if(strcmp((yyvsp[-3].sval), "MIN") == 0)
	  {
	    if((yyvsp[-1].ptr_array)->len < 1)
	      yyerror(engine, "wrong number of arguments for MIN");

	    (yyval.expr_ptr) = expr_new_func_min((yyvsp[-1].ptr_array));
	    g_ptr_array_unref((yyvsp[-1].ptr_array));
	  }
        else if(strcmp((yyvsp[-3].sval), "MAX") == 0)
	  {
	    if((yyvsp[-1].ptr_array)->len < 1)
	      yyerror(engine, "wrong number of arguments for MAX");

	    (yyval.expr_ptr) = expr_new_func_max((yyvsp[-1].ptr_array));
	    g_ptr_array_unref((yyvsp[-1].ptr_array));
	  }
        else if(strcmp((yyvsp[-3].sval), "ROUND") == 0)
	  {
	    if((yyvsp[-1].ptr_array)->len != 1)
	      yyerror(engine, "wrong number of arguments for ROUND");

	    (yyval.expr_ptr) = expr_new_func_round((yyvsp[-1].ptr_array));
	    g_ptr_array_unref((yyvsp[-1].ptr_array));
	  }
        else if(strcmp((yyvsp[-3].sval), "LIMIT") == 0)
	  {
	    if((yyvsp[-1].ptr_array)->len != 3)
	      yyerror(engine, "wrong number of arguments for LIMIT");

	    (yyval.expr_ptr) = expr_new_func_limit((yyvsp[-1].ptr_array));
	    g_ptr_array_unref((yyvsp[-1].ptr_array));
	  }
        else if(strcmp((yyvsp[-3].sval), "EVAL") == 0)
	  {
	    if((yyvsp[-1].ptr_array)->len != 1)
	      yyerror(engine, "wrong number of arguments for EVAL");

	    (yyval.expr_ptr) = expr_new_func_eval((yyvsp[-1].ptr_array));
	    g_ptr_array_unref((yyvsp[-1].ptr_array));
	  }
        else if(strcmp((yyvsp[-3].sval), "CHANGED") == 0)
	  {
	    if((yyvsp[-1].ptr_array)->len < 1)
	      yyerror(engine, "wrong number of arguments for CHANGED");

	    (yyval.expr_ptr) = expr_new_func_changed((yyvsp[-1].ptr_array));
	    g_ptr_array_unref((yyvsp[-1].ptr_array));
	  }
	else
	  {
	    yyerror(engine, "Unknown function");
	  }

	free((yyvsp[-3].sval));
      }
#line 1935 "rules.c"
    break;

  case 38: /* argument_list: %empty  */
#line 353 "rules.y"
    { }
#line 1941 "rules.c"
    break;

  case 39: /* argument_list: expr  */
#line 354 "rules.y"
           {
      (yyval.ptr_array) = g_ptr_array_new_full(0, (GDestroyNotify) expr_unref);
      g_ptr_array_add((yyval.ptr_array), (yyvsp[0].expr_ptr));
    }
#line 1950 "rules.c"
    break;

  case 40: /* argument_list: argument_list ',' expr  */
#line 358 "rules.y"
                             {
      g_ptr_array_add((yyval.ptr_array), (yyvsp[0].expr_ptr));
    }
#line 1958 "rules.c"
    break;


#line 1962 "rules.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (engine, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, engine);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, engine);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (engine, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, engine);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, engine);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 363 "rules.y"
 

/* Additional C code */
void yyerror (Engine *engine, char const *s)
{
  fprintf(stderr, "%s: error parsing rules file at line %d: %s\n", PACKAGE, yylineno, s);
}
