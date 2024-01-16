# meta_types

Component that is included some compile time type operation for meta programming that were needed to me and I could not found in std

This file will be extended if I will found new cases that are useful for me and not found (actually, it is for my own usage)

Supported from C++17 but C++20 can give some benefits
Also compile time unit tests are included in the module

## const_v

Transform the value to real const (turn into type property)
Can be useful when non-type template parameters are not allowed/not beautiful
For example, with this is possible to write overloading operator with constexpr parameters

```cpp
// Create example variable
static constexpr auto var = const_v<0x12345678UL>;

// Using a concept to define the function which accept any const_v variable
template<typename Param>
requires const_value<Param> // If type is not ConstVal - compiler and IntelliSense error
static constexpr void Function(const Param) {
    // Write your code here
    static_assert(0x12345678UL == Param::value, "Oh no, incorrect value!");

    // And perform some compilation...
}

// Also trait can be used for enable-if semantics until C++20
static_assert(is_const_v<var>, "Is var const_v?!!");

// Can call the function
Function(const_v<var>)
Function(const_v<'Any value of any type'>);
```

## var_pack

Class supports next compile-time operation for variadic pack (all types should be unique):

- Search that all types of '<typename ...Args>' are belonging to predefined type list
- Search that all types of '<const auto ...args>' are belonging to predefined type list
- Ensure that is all types are unique'<typename ...Args>'
- Ensure that is all types are unique'<const auto ...args>'
- Extract the value according to a type from the given pack (if no type found - return default type value)

For specific example please check UT section but I will provide some generics:

Example for '<typename ...Args>' pack

```cpp
// Define some types
enum class Port { PA, PB };
enum class Pin { Pin_0, Pin_1, Pin_2, Pin_3, Pin_4, Pin_5, Pin_6, Pin_7, Pin_8, Pin_9, Pin_10, Pin_11, Pin_12, Pin_13, Pin_14, Pin_15 };
enum class Mode : uint32_t { Input, Output, Alternate, Analog };
enum class Type : uint32_t { PushPull, OpenDrain };
enum class Speed : uint32_t { Low, Medium, High = 0b11 };
enum class Pull : uint32_t { None, Up, Down };
enum class Alternative : uint32_t { AF0, AF1, AF2, AF3, AF4, AF5, AF6, AF7 };

// Consteval structure
struct GpioConfig {
  const Port cm_Port;
  const Pin cm_Pin;
  const Mode cm_Mode;
  const Type cm_Type;
  const Speed cm_Speed;
  const Pull cm_Pull;
  const Alternative cm_AlternativeFunction;

  // Consteval constructor that is accept three mandatory and some additional parameters (from 0 to 4)
  template <typename... AddParams>
  requires iso::meta_type::var_pack::is_types_unique_v<AddParams...> && // Check that all given types are unique
               iso::meta_type::var_pack::is_type_list<Type, Speed, Pull, Alternative>::contains_v<AddParams...> // Check that all given params is our auxiliary
  consteval GpioConfig(const Port p_Port, const Pin p_Pin, const Mode p_Mode, const AddParams... p_AddParams)
      : cm_Port(p_Port), cm_Pin(p_Pin), cm_Mode(p_Mode), cm_Type(iso::meta_type::var_pack::type<Type>::get(p_AddParams...)),
        cm_Speed(iso::meta_type::var_pack::type<Speed>::get(p_AddParams...)), cm_Pull(iso::meta_type::var_pack::type<Pull>::get(p_AddParams...)),
        cm_AlternativeFunction(iso::meta_type::var_pack::type<Alternative>::get(p_AddParams...)) {} // Get all auxiliary according to the ype
};
```

Example for '<const auto ...args>' pack

```cpp
// Define some types
enum class Port { P0, P1 };
enum class Pin { Pin_0, ... Pin_31 };
enum class Mode { Input, Output };
enum class Input { Connect, Disconnect };
enum class Pull { No, Down, Up = 3 };
enum class Drive { SOS1, HOS1, SOH1, HOH1, DOS1, DOH1, SOD1, HOD1, EDE1 = 11 };
enum class Sense { Disabled, High = 2, Low };
enum class McuSel { AppMCU, NetworkMCU, Peripheral, TND };

// Class accept three mandatory and some additional parameters (from 0 to 5)
template <const Peripherals<Port> port, const Pin pin, const Mode mode, const auto... params>
requires  (var_pack::is_types_val_unique_v(params...)) && // Check that all given types are unique
        (var_pack::is_type_val_list<Input, Pull, Drive, Sense, McuSel>::contains_v(params...)) // Check that all given params is our auxiliary
class Gpio { 
  static constexpr auto INPUT = var_pack::type<Input, Input::Disconnect>::get(params...); // Changed the default value that will be returned is not found
  static constexpr auto PULL = var_pack::type<Pull>::get(params...);
  static constexpr auto DRIVE = var_pack::type<Drive>::get(params...);
  static constexpr auto SENSE = var_pack::type<Sense>::get(params...);
  static constexpr auto MCUSEL = var_pack::type<McuSel>::get(params...);
};
```
