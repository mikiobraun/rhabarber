/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LRP = 258,
     RRP = 259,
     SYMBOL = 260,
     STRING = 261,
     INT = 262,
     REAL = 263,
     BOOL = 264,
     FN = 265,
     THIS = 266,
     THAT = 267,
     PARENT = 268,
     VOID = 269,
     BS = 270,
     WSL = 271,
     RSP = 272,
     LSP = 273,
     SEMIC = 274,
     IMPORT = 275,
     CATCH = 276,
     TRY = 277,
     FOR = 278,
     WHILE = 279,
     IF = 280,
     ELSE = 281,
     RETURN = 282,
     BREAK = 283,
     DELIVER = 284,
     THROW = 285,
     ENTAILS = 286,
     COMMA = 287,
     SNATCH = 288,
     DIVIDEASSIGN = 289,
     TIMESASSIGN = 290,
     MINUSASSIGN = 291,
     PLUSASSIGN = 292,
     ASSIGN = 293,
     SYMASSIGN = 294,
     COLON = 295,
     MAPS = 296,
     NOT = 297,
     OR = 298,
     AND = 299,
     NOTEQUAL = 300,
     EQUAL = 301,
     GREATEREQUAL = 302,
     GREATER = 303,
     LESSEQUAL = 304,
     LESS = 305,
     MINUS = 306,
     PLUS = 307,
     DIVIDE = 308,
     TIMES = 309,
     MINUSMINUS = 310,
     PLUSPLUS = 311,
     PRE = 312,
     NEG = 313,
     NOBINDDOT = 314,
     QUEST = 315,
     DOT = 316,
     RCP = 317,
     LCP = 318
   };
#endif
/* Tokens.  */
#define LRP 258
#define RRP 259
#define SYMBOL 260
#define STRING 261
#define INT 262
#define REAL 263
#define BOOL 264
#define FN 265
#define THIS 266
#define THAT 267
#define PARENT 268
#define VOID 269
#define BS 270
#define WSL 271
#define RSP 272
#define LSP 273
#define SEMIC 274
#define IMPORT 275
#define CATCH 276
#define TRY 277
#define FOR 278
#define WHILE 279
#define IF 280
#define ELSE 281
#define RETURN 282
#define BREAK 283
#define DELIVER 284
#define THROW 285
#define ENTAILS 286
#define COMMA 287
#define SNATCH 288
#define DIVIDEASSIGN 289
#define TIMESASSIGN 290
#define MINUSASSIGN 291
#define PLUSASSIGN 292
#define ASSIGN 293
#define SYMASSIGN 294
#define COLON 295
#define MAPS 296
#define NOT 297
#define OR 298
#define AND 299
#define NOTEQUAL 300
#define EQUAL 301
#define GREATEREQUAL 302
#define GREATER 303
#define LESSEQUAL 304
#define LESS 305
#define MINUS 306
#define PLUS 307
#define DIVIDE 308
#define TIMES 309
#define MINUSMINUS 310
#define PLUSPLUS 311
#define PRE 312
#define NEG 313
#define NOBINDDOT 314
#define QUEST 315
#define DOT 316
#define RCP 317
#define LCP 318




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

