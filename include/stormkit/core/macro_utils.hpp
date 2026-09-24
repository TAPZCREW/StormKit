// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_MACRO_UTILS_HPP
#define STORMKIT_MACRO_UTILS_HPP

#define STORMKIT_VAARGS_HEAD(head, ...) head
#define STORMKIT_VAARGS_TAIL(head, ...) __VA_ARGS__
#define STORMKIT_HAS_VAARGS(...)        STORMKIT_VAARGS_HEAD(__VA_OPT__(1, ) 0)

#define STORMKIT_STRINGIFY_DETAILS(x)  #x
#define STORMKIT_STRINGIFY(x)          STORMKIT_STRINGIFY_DETAILS(x)
#define STORMKIT_PRAGMA_FROM_STRING(x) _Pragma(STORMKIT_STRINGIFY(x))

#define STORMKIT_CONCAT(a, b)         STORMKIT_CONCAT_DETAILS(a, b)
#define STORMKIT_CONCAT_DETAILS(a, b) a##b

#define STORMKIT_UNIQUE_NAME(base) STORMKIT_CONCAT(base, __COUNTER__)

#define STORMKIT_TUPLE_APPEND(tuple, el)       (STORMKIT_TUPLE_APPEND_IMPL_UNPACK tuple el)
#define STORMKIT_TUPLE_APPEND_IMPL_UNPACK(...) __VA_ARGS__ __VA_OPT__(, )

// STORMKIT_IIF doesn't work but IIF work WTF ????
#define STORMKIT_IIF(value)           STORMKIT_CONCAT(STORMKIT_IIF_, value)
#define STORMKIT_IIF_0(true_, false_) true_
#define STORMKIT_IIF_1(true_, false_) false_
#define IIF(value)                    STORMKIT_CONCAT(IIF_, value)
#define IIF_1(true_, false_)          true_
#define IIF_0(true_, false_)          false_

#define STORMKIT_INC(x) STORMKIT_CONCAT(INC_, x)
#define STORMKIT_INC_0  1
#define STORMKIT_INC_1  2
#define STORMKIT_INC_2  3
#define STORMKIT_INC_3  4
#define STORMKIT_INC_4  5
#define STORMKIT_INC_5  6
#define STORMKIT_INC_6  7
#define STORMKIT_INC_7  8
#define STORMKIT_INC_8  9
#define STORMKIT_INC_9  9

#define STORMKIT_DEC(x) STORMKIT_CONCAT(STORMKIT_DEC_, x)
#define STORMKIT_DEC_0  0
#define STORMKIT_DEC_1  0
#define STORMKIT_DEC_2  1
#define STORMKIT_DEC_3  2
#define STORMKIT_DEC_4  3
#define STORMKIT_DEC_5  4
#define STORMKIT_DEC_6  5
#define STORMKIT_DEC_7  6
#define STORMKIT_DEC_8  7
#define STORMKIT_DEC_9  8

#define STORMKIT_CHECK_N(x, n, ...) n
#define STORMKIT_CHECK(...)         STORMKIT_CHECK_N(__VA_ARGS__, 0, )
#define STORMKIT_PROBE(x)           x, 1,

#define STORMKIT_NOT(x) STORMKIT_CHECK(STORMKIT_CONCAT(STORMKIT_NOT_, x))
#define STORMKKIT_NOT_0 STORMKIT_PROBE(~)

#define STORMKIT_BOOL(x) STORMKIT_COMPL(STORMKIT_NOT(x))
#define STORMKIT_IF(c)   STORMKIT_IIF(STORMKIT_BOOL(c))

#define STORMKIT_EAT(...)
#define STORMKIT_EXPAND(...) __VA_ARGS__
#define STORMKIT_WHEN(c)     STORMKIT_IF(c)(STORMKIT_EXPAND, STORMKIT_EAT)

#define STORMKIT_EMPTY()
#define STORMKIT_DEFER(id)     id STORMKIT_EMPTY()
#define STORMKIT_OBSTRUCT(...) __VA_ARGS__ STORMKIT_DEFER(STORMKIT_EMPTY)()

#define STORMKIT_DELAY STORMKIT_DELAY_IMPL_NOTHING
#define STORMKIT_DELAY_IMPL_NOTHING()

#define STORMKIT_EVAL(...)  STORMKIT_EVAL1(STORMKIT_EVAL1(STORMKIT_EVAL1(__VA_ARGS__)))
#define STORMKIT_EVAL1(...) STORMKIT_EVAL2(STORMKIT_EVAL2(STORMKIT_EVAL2(__VA_ARGS__)))
#define STORMKIT_EVAL2(...) STORMKIT_EVAL3(STORMKIT_EVAL3(STORMKIT_EVAL3(__VA_ARGS__)))
#define STORMKIT_EVAL3(...) STORMKIT_EVAL4(STORMKIT_EVAL4(STORMKIT_EVAL4(__VA_ARGS__)))
#define STORMKIT_EVAL4(...) STORMKIT_EVAL5(STORMKIT_EVAL5(STORMKIT_EVAL5(__VA_ARGS__)))
#define STORMKIT_EVAL5(...) __VA_ARGS__
#define STORMKIT_REPEAT(count, macro, ...)                                                  \
    STORMKIT_WHEN(count)                                                                    \
    (STORMKIT_OBSTRUCT(STORMKIT_REPEAT_INDIRECT)()(STORMKIT_DEC(count), macro, __VA_ARGS__) \
       STORMKIT_OBSTRUCT(macro)(STORMKIT_DEC(count), __VA_ARGS__))

#define STORMKIT_REPEAT_INDIRECT() STORMKIT_REPEAT

#define STORMKIT_WHILE(pred, op, ...) \
    STORMKIT_IF(pred(__VA_ARGS__))    \
    (STORMKIT_OBSTRUCT(STORMKIT_WHILE_INDIRECT)()(pred, op, op(__VA_ARGS__)), __VA_ARGS__)
#define STORMKIT_WHILE_INDIRECT() STORMKIT_WHILE

#define STORMKIT_FOR_EACH_COMBINATION_IMPL_CALL(fn, args) fn args

#define STORMKIT_FOR_EACH_COMBINATION_IMPL(fn, args, ...)                             \
    IIF(STORMKIT_HAS_VAARGS(__VA_ARGS__))(STORMKIT_FOR_EACH_COMBINATION_IMPL_RECURSE, \
                                          STORMKIT_FOR_EACH_COMBINATION_IMPL_CALL)(fn, args __VA_OPT__(, __VA_ARGS__))

#define STORMKIT_FOR_EACH_COMBINATION_IMPL_RECURSE(fn, args, tuple, ...)                           \
    IIF(STORMKIT_HAS_VAARGS tuple)(STORMKIT_FOR_EACH_COMBINATION_IMPL_RECURSE_APPLY, STORMKIT_EAT) \
      STORMKIT_DELAY()(fn, args, tuple __VA_OPT__(, __VA_ARGS__))

#define STORMKIT_FOR_EACH_COMBINATION_IMPL_RECURSE_APPLY(fn, args, tuple, ...)                              \
    STORMKIT_FOR_EACH_COMBINATION_IMPL                                                                      \
    STORMKIT_DELAY()(fn, STORMKIT_TUPLE_APPEND(args, STORMKIT_VAARGS_HEAD tuple) __VA_OPT__(, __VA_ARGS__)) \
                                                                                                            \
      STORMKIT_FOR_EACH_COMBINATION_IMPL                                                                    \
      STORMKIT_DELAY()(fn, args, (STORMKIT_VAARGS_TAIL tuple)__VA_OPT__(, __VA_ARGS__))

#define STORMKIT_FOR_EACH_COMBINATION(fn, ...) STORMKIT_EVAL(STORMKIT_FOR_EACH_COMBINATION_IMPL(fn, () __VA_OPT__(, __VA_ARGS__)))

#endif
