/**
 * @file parameters_pack.hpp
 * @author Ivan Sobchuk (i.a.sobchuk.1994@gmail.com)
 * @brief Meta types for compile-time programming
 *
 * @date 2024-01-03
 *
 * @copyright Ivan Sobchuk (c) 2024
 *
 * License Apache 2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
#pragma once

// Suppoted since C++17 as a last fully-supported standard for gcc and clang
static_assert((__cplusplus >= 201703L), "Supported only with C++17 and newer!");

#if __cpp_concepts
#include <concepts>
#endif

// General namespace for the module
namespace iso::meta_type {

/**
 * @brief Template to convert value to constexpr
 *
 * @tparam param: Value that will be casted to constexpr
 */
template <const auto param> struct ConstValue final {
  using type = decltype(param);
  static constexpr auto value = param;
  struct ConstValueT {
    using type = void;
  };
};

/**
 * @brief Template to convert reference to constexpr
 *
 * @tparam param: Reference that will be casted to constexpr
 */
template <const auto &param> struct ConstReference final {
  using type = decltype(param);
  static constexpr auto &value = param;
  struct ConstReferenceT {
    using type = void;
  };
};

// Cast to the real const value
template <const auto value> inline constexpr auto const_v = ConstValue<value>{};
// Cast to the real const type
template <const auto value> using const_t = ConstValue<value>;

// Cast to the real const reference
template <const auto &value> inline constexpr auto const_ref_v = ConstReference<value>{};
// Cast to the real const reference
template <const auto &value> using const_ref_t = ConstReference<value>;

// Concept for C++20 to check type for ConstValue
#ifdef __cpp_concepts
template <typename T>
concept const_value = requires(T) {
  typename T::ConstValueT;
  T::value;
  typename T::type;
};

template <typename T, typename Type>
concept const_value_of_type = const_value<T> && std::same_as<Type, typename T::type>;

// Concept for C++20 to check type for ConstReference
template <typename T>
concept const_reference = requires(T) {
  typename T::ConstReferenceT;
  T::value;
  typename T::type;
};

template <typename T, typename Type>
concept const_reference_of_type = const_reference<T> && std::same_as<Type, typename T::type>;

// Array operation
template <typename T>
concept array = std::is_array_v<T>;

template <typename T, typename Type>
concept array_of_type = std::is_array_v<T> && std::same_as<Type, T>;
#endif

// Type traits for SFINAE to check type for ConstValue
template <typename T, typename U = void> struct is_const {
  static constexpr auto value = false;
};

template <typename T> struct is_const<T, typename T::ConstValueT::type> {
  static constexpr auto value = true;
};

template <typename T> inline constexpr auto is_const_v = is_const<T>::value;

/**
 * @brief Class that implements compile time template variadic pack analysis
 *        Supposed that all types inside variadic pack are unique
 *
 * @note  Class supports next compile-time operation for variadic pack :
 *        - Search that all types of '<typename ...Args>' are belonging to predefined type list
 *        - Search that all types of '<const auto ...args>' are belonging to predefined type list
 *        - Ensure that is all types are unique'<typename ...Args>'
 *        - Ensure that is all types are unique'<const auto ...args>'
 *        - Extract the value according to a type from the given pack (if no type found - return default type value)
 *        Please, check specific methods, unit tests and Readme for the usage example
 */
class var_pack {
  template <typename T, typename U> struct is_same {
    static constexpr bool value = false;
  };

  template <typename T> struct is_same<T, T> {
    static constexpr bool value = true;
  };

  template <typename T, typename U> static constexpr bool is_same_v = is_same<T, U>::value;

  template <typename... Types> class is_types_unique {
    template <typename First, typename... Rest> struct duplicate_type {
      static constexpr bool value = (!is_same_v<First, Rest> && ...) && duplicate_type<Rest...>::value;
    };

    template <typename First> struct duplicate_type<First> {
      static constexpr bool value = true;
    };

  public:
    static constexpr bool value = []() {
      if constexpr (sizeof...(Types)) {
        return duplicate_type<Types...>::value;
      } else {
        return true;
      }
    }();
  };

  struct duplicate {
    template <typename T> inline static constexpr bool is_same_v(const T, const T) { return true; }
    template <typename T, typename U> inline static constexpr bool is_same_v(const T, const U) { return false; }

    inline static constexpr bool duplicate_types_val() { return true; }
    template <typename First> inline static constexpr bool duplicate_types_val(const First) { return true; }
    template <typename First, typename... Rest> inline static constexpr bool duplicate_types_val(const First first, const Rest... rest) {
      return (!is_same_v(first, rest) && ...) && duplicate_types_val(rest...);
    }
  };

  template <typename TypeFirst, typename... TypesRest> struct contains_list {
    inline static constexpr bool is_parameter_inside(const TypeFirst) { return true; }
    template <typename Value> inline static constexpr bool is_parameter_inside(const Value value) {
      return contains_list<TypesRest...>::is_parameter_inside(value);
    }

    inline static constexpr bool contains() { return true; }
    template <typename First, typename... Rest> inline static constexpr bool contains(const First first, const Rest... rest) {
      return is_parameter_inside(first) ? contains(rest...) : false;
    }
  };

  template <typename TypeFirst> struct contains_list<TypeFirst> {
    inline static constexpr bool is_parameter_inside(const TypeFirst) { return true; }
    template <typename Value> inline static constexpr bool is_parameter_inside(const Value) { return false; }

    inline static constexpr bool contains() { return true; }
    template <typename First, typename... Rest> inline static constexpr bool contains(const First first, const Rest... rest) {
      return is_parameter_inside(first) ? contains(rest...) : false;
    }
  };

public:
  /**
   * @brief Search that all types of '<typename ...Args>' are belonging to predefined type list
   *
   * @note   Usage guideline: var_pack::is_type_list<'your predefined types'>::contains_v<'Args...'>
   *
   * @tparam TypeFirst First type in the list
   * @tparam TypeRest  Rest types from the list
   */
  template <typename TypeFirst, typename... TypeRest> class is_type_list {
  protected:
    template <typename T> struct is_parameter_inside : public is_type_list<TypeRest...> {
      static constexpr bool value = is_same_v<T, TypeFirst> ? true : is_type_list<TypeRest...>::template is_parameter_inside<T>::value;
    };

  private:
    template <typename ParamFirst, typename... ParamRest> struct contains {
      static constexpr bool value = is_parameter_inside<ParamFirst>::value ? contains<ParamRest...>::value : false;
    };

    template <typename ParamFirst> struct contains<ParamFirst> {
      static constexpr bool value = is_parameter_inside<ParamFirst>::value ? true : false;
    };

  public:
    template <typename... Params>
    static constexpr bool contains_v = []() {
      if constexpr (sizeof...(Params)) {
        return contains<Params...>::value;
      } else {
        return true;
      }
    }();
  };

  /**
   * @brief Search that all types of '<typename ...Args>' are belonging to predefined type list
   *        Overloading for one type to stop infinite recursion
   *
   * @tparam TypeFirst The last list type
   */
  template <typename TypeFirst> class is_type_list<TypeFirst> {
  protected:
    template <typename T> struct is_parameter_inside {
      static constexpr bool value = is_same_v<T, TypeFirst> ? true : false;
    };

  private:
    template <typename ParamFirst, typename... ParamRest> struct contains {
      static constexpr bool value = is_parameter_inside<ParamFirst>::value ? contains<ParamRest...>::value : false;
    };

    template <typename ParamFirst> struct contains<ParamFirst> {
      static constexpr bool value = is_parameter_inside<ParamFirst>::value ? true : false;
    };

  public:
    template <typename... Params>
    static constexpr bool contains_v = []() {
      if constexpr (sizeof...(Params)) {
        return contains<Params...>::value;
      } else {
        return true;
      }
    }();
  };

  template <typename... Types> static constexpr bool is_types_unique_v = is_types_unique<Types...>::value;

  /**
   * @brief Ensure that is all types are unique'<const auto... args>'
   *
   * @note   Usage guideline: var_pack::is_types_val_unique_v('args...')
   *
   */
  inline static constexpr bool is_types_val_unique_v() { return true; }
  template <typename First> inline static constexpr bool is_types_val_unique_v(const First) { return true; }
  template <typename First, typename... Rest> inline static constexpr bool is_types_val_unique_v(const First first, const Rest... rest) {
    return duplicate::duplicate_types_val(first, rest...);
  }

  /**
   * @brief Search that all types of '<const auto... args>' are belonging to predefined type list
   *
   * @note   Usage guideline: var_pack::is_type_val_list<'your predefined types'>::contains_v('args...')
   *
   * @tparam TypeFirst First type in the list
   * @tparam TypeRest  Rest types from the list
   */
  template <typename TypeFirst, typename... TypesRest> struct is_type_val_list {
    inline static constexpr bool contains_v() { return true; }
    template <typename First, typename... Rest> inline static constexpr bool contains_v(const First first, const Rest... rest) {
      return contains_list<TypeFirst, TypesRest...>::contains(first, rest...);
    }
  };

  /**
   * @brief Extract the value according to a type from the given pack
   *
   * @note   Usage guideline: var_pack::type<'type of value', '[auxilary] not standard default'>::get('args...')
   *
   * @tparam TypeFirst First type in the list
   * @tparam TypeRest  Rest types from the list
   */
  template <typename Type, const Type defaultValue = Type{}> struct type {
    inline static constexpr Type get() { return defaultValue; }
    inline static constexpr Type get(const Type first) { return first; }
    template <typename First> inline static constexpr Type get(const First) { return defaultValue; }
    template <typename... Rest> inline static constexpr Type get(const Type first, const Rest...) { return first; }
    template <typename First, typename... Rest> inline static constexpr Type get(const First, const Rest... rest) { return get(rest...); }
  };
};

#ifdef __cpp_concepts
// Concepts to check that all types are unique
template <typename... Types>
concept types_unique = var_pack::is_types_unique_v<Types...>;
template <const auto... Values>
concept types_val_unique = var_pack::is_types_val_unique_v<Values...>;
#endif

#ifdef ISO_META_TYPE_UNITTEST
// Unit test for the module. As it is compile time - can be performed during every compilation
namespace unit_tests {
// Test Types
struct TestType1 {};
struct TestType2 {};
struct TestType3 {};
enum class TestType4 : unsigned { TestValue0 = 0x5667U, TestValue1 = 0xA100U, TestValue2 = 0x7832AD01UL };
enum class TestType5 : int { TestValue0 = -777, TestValue1 = 256901 };
enum class TestType6 { TestValue0, TestValue1, TestValue2, TestValue3 };
using TestType7 = bool;
using TestType8 = unsigned long;
using TestType9 = unsigned;

template <typename... Args> inline constexpr bool unique_args(const Args...) { return var_pack::is_types_unique_v<Args...>; }
template <typename... Args> inline constexpr bool unique_args2(const Args... args) { return var_pack::is_types_val_unique_v(args...); }
template <typename... Args> inline constexpr bool types456inside(const Args...) {
  return var_pack::is_type_list<TestType4, TestType5, TestType6>::contains_v<Args...>;
}

#ifdef __cpp_concepts
template <typename... Args>
requires types_unique<Args...> && (var_pack::is_type_list<TestType1, TestType2, TestType3>::contains_v<Args...>)
inline constexpr bool test_with_requires123(const Args...) {
  return true;
}
#endif

class Test {
  // Test for no type repetition in the parameter pack
  static_assert(var_pack::is_types_unique_v<TestType1, TestType2, TestType3, TestType6>, "Check the unique list 1");
  static_assert(var_pack::is_types_unique_v<TestType1, TestType2, TestType3, TestType6, TestType4, TestType9, TestType7>, "Check the unique list 2");
  static_assert(!var_pack::is_types_unique_v<TestType1, TestType2, TestType3, TestType6, TestType4, TestType3, TestType7>,
                "Check not unique in the middle 1");
  static_assert(!var_pack::is_types_unique_v<TestType1, TestType2, TestType3, TestType6, TestType2, TestType8, TestType7>,
                "Check not unique in the middle 2");
  static_assert(!var_pack::is_types_unique_v<TestType1, TestType6, TestType4, TestType1, TestType7>, "Check not unique in at the start 1");
  static_assert(!var_pack::is_types_unique_v<TestType4, TestType6, TestType4, TestType5, TestType7>, "Check not unique in at the start 2");
  static_assert(!var_pack::is_types_unique_v<TestType1, TestType2, TestType6, TestType6>, "Check not unique in at the end 1");
  static_assert(!var_pack::is_types_unique_v<TestType1, TestType2, TestType3, TestType7, TestType4, TestType9, TestType7>, "Check the unique end 2");
  static_assert(unique_args(TestType1{}, TestType4::TestValue0, TestType3{}, TestType6::TestValue0), "Check the unique list with function 1");
  static_assert(!unique_args(TestType1{}, TestType9{}, TestType3{}, TestType6{}, TestType9{}, TestType7{}), "Check the unique list with function2");
  static_assert(unique_args(), "Check the empty unique list with function");
#ifdef __cpp_concepts
  static_assert(types_unique<TestType1, TestType2, TestType3, TestType6>, "Check the unique list with concept 1");
  static_assert(!types_unique<TestType1, TestType7, TestType3, TestType4, TestType9, TestType7>, "Check the unique list with concept 2");
#endif

  // Test for no type lists (that there are required types)
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType6>::contains_v<TestType1>, "Check a type at the start of the list");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType9, TestType3, TestType6>::contains_v<TestType9>,
                "Check a type at the middle of the list");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType9, TestType3, TestType6>::contains_v<TestType6>,
                "Check a type at the end of the list");
  static_assert(!var_pack::is_type_list<TestType1, TestType2, TestType9, TestType3, TestType6>::contains_v<TestType7>, "Check a wrong type");

  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType1, TestType4,
                                                                                                                                TestType7, TestType8>,
                "Check types list of types");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType8, TestType7,
                                                                                                                                TestType1, TestType4>,
                "Check reverse types list of types");
  static_assert(!var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8,
                                        TestType9>::contains_v<TestType5, TestType4, TestType7, TestType8>,
                "Check types list of types with no type at start");
  static_assert(!var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8,
                                        TestType9>::contains_v<TestType1, TestType5, TestType7, TestType8>,
                "Check types list of types with no type at middle");
  static_assert(!var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8,
                                        TestType9>::contains_v<TestType1, TestType4, TestType7, TestType5>,
                "Check types list of types with no type at end");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType9, TestType7,
                                                                                                                                TestType1, TestType4>,
                "Check edges 1");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType8, TestType9,
                                                                                                                                TestType1, TestType4>,
                "Check edges 2");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType8, TestType3,
                                                                                                                                TestType1, TestType9>,
                "Check edges 3");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType1, TestType7,
                                                                                                                                TestType3, TestType4>,
                "Check edges 4");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType8, TestType1,
                                                                                                                                TestType3, TestType4>,
                "Check edges 5");
  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3, TestType4, TestType7, TestType8, TestType9>::contains_v<TestType8, TestType3,
                                                                                                                                TestType7, TestType1>,
                "Check edges 6");
  static_assert(types456inside(TestType4::TestValue2), "Check with function 1");
  static_assert(types456inside(TestType5::TestValue0), "Check with function 2");
  static_assert(types456inside(TestType6::TestValue1), "Check with function 3");
  static_assert(!types456inside(TestType7{}), "Check with function 4");
  static_assert(!types456inside(TestType7{}, TestType5::TestValue0, TestType6::TestValue3), "Check with function 5");
  static_assert(!types456inside(TestType4::TestValue0, TestType7{}, TestType6::TestValue3), "Check with function 6");
  static_assert(!types456inside(TestType4::TestValue1, TestType5::TestValue0, TestType7{}), "Check with function 7");
  static_assert(types456inside(TestType4::TestValue1, TestType5::TestValue0, TestType6::TestValue3), "Check with function 8");
  static_assert(types456inside(), "Check with function 9");

  static_assert(var_pack::is_type_list<TestType1, TestType2, TestType3>::contains_v<TestType1, TestType3>);

#ifdef __cpp_concepts
  static_assert(test_with_requires123(TestType1{}, TestType3{}), "Test for requires expression 1");
  static_assert(test_with_requires123(TestType2{}), "Test for requires expression 2");
  static_assert(test_with_requires123(), "Test for requires expression 3");
#endif

  static_assert((TestType4::TestValue2 == var_pack::type<TestType4>::get(true, TestType4::TestValue2, 367UL)), "Test for get_by_type expression 1");
  static_assert((static_cast<TestType4>(0) == var_pack::type<TestType4>::get(true, -1, 367U)), "Test for get_by_type expression 2");
  static_assert((TestType4::TestValue1 == var_pack::type<TestType4, TestType4::TestValue1>::get(true, -1, 367U)),
                "Test for get_by_type expression 3");

  static_assert(var_pack::is_types_val_unique_v(TestType4::TestValue2, TestType5::TestValue1, true), "Test for get_by_type expression 1");
  static_assert(!var_pack::is_types_val_unique_v(TestType4::TestValue2, false, TestType5::TestValue1, true), "Test for get_by_type expression 2");
  static_assert(var_pack::is_types_val_unique_v(TestType4::TestValue2, TestType5::TestValue1, true, -36, 5743737U, TestType6::TestValue3),
                "Test for get_by_type expression 3");

  static_assert(unique_args2(TestType1{}, TestType4::TestValue0, TestType3{}, TestType6::TestValue0), "Check the unique list with function 1");
  static_assert(!unique_args2(TestType1{}, TestType9{}, TestType3{}, TestType6{}, TestType9{}, TestType7{}), "Check the unique list with function2");
  static_assert(unique_args2(), "Check the empty unique list with function");

  static_assert(var_pack::is_type_val_list<signed, TestType4, bool, unsigned, long>::contains_v(TestType4::TestValue2, -56836L),
                "Check type list with params 1");
  static_assert(!var_pack::is_type_val_list<signed, TestType4, bool, unsigned, long>::contains_v(TestType4::TestValue2, -56836L, 745983548UL),
                "Check type list with params 2");
  static_assert(var_pack::is_type_val_list<signed, TestType4, bool, unsigned, long>::contains_v(), "Check type list with params 3");
};
}; // namespace unit_tests
#endif

} // namespace iso::meta_type