#ifndef BLOFELD_COMPARTMENT_TYPES_H
#define BLOFELD_COMPARTMENT_TYPES_H

namespace blofeld
{

  enum class ModelType
  {
    Deterministic,  // Uses double and simple maths
    Stochastic      // Uses int and random sampling
  };

  enum class ContainerType
  {
    Disabled,       // Removed - compiles to nothing: n=0
    Array,          // Fixed size (including 1, but not zero): n>0
    InplaceVector, // Emulation of c++26 inplace_vector i.e. stack-based, dynamic up to max size: n>0 (=max)
    Vector,         // Heap-based, dynamic - s_ctype.n is ignored: n=1
    BirthDeath     // Fixed size of 1 and allows negative values (for birth/death) - n=0 or n=1
  };

  enum class CarryType
  {
    Sequential,       // Traditional carry-through over sequential time points
    Immediate,        // Carry-through within a single time point
    None              // For n==0 or n==1
  };

  // Literal type
  struct CompartmentInfo
  {
    int const n;
    ContainerType const container_type;
    CarryType const carry_type;    
  };
  
  // Helper function (with default arguments - note that we use int not size_t for n)
  consteval CompartmentInfo make_compartment_info(int const n = 1, ContainerType const cont_type = ContainerType::Array, CarryType const carry_type = CarryType::Sequential)
  {
    // Check n is valid (specified as int to avoid using unsigned types):
    if (n < 0) throw("Invalid n < 0");
    
    // Allow shortcut of specifying n==0 with default cont_type==array to mean disable:
    if (n == 0 && (cont_type == ContainerType::Array || cont_type == ContainerType::Disabled)) {
      return CompartmentInfo {
        .n = 0,
        .container_type = ContainerType::Disabled,
        .carry_type = CarryType::None
      };    
    }
    
    // Checks:
    if (cont_type == ContainerType::Disabled) throw("For ContainerType::disabled n must be equal to 0");
    if (cont_type == ContainerType::BirthDeath && n!=1) throw("For ContainerType::BirthDeath n must be equal to 1");
    // Note: with n=1 InplaceVector and Array are almost identical, but allow both for bug hunting etc
    if (cont_type != ContainerType::Vector && n==0) throw("For ContainerType::Array and ContainerType::InplaceVector n must be greater than 0");
        
    // For Array or InplaceVector or BirthDeath with n==1 we can ignore carry_type:
    if (n == 1 && cont_type != ContainerType::Vector) {
      return CompartmentInfo {
        .n = n,
        .container_type = cont_type,
        .carry_type = CarryType::None
      };    
    }
    
    // Otherwise we can use what we were given:
    return CompartmentInfo {
      .n = n,
      .container_type = cont_type,
      .carry_type = carry_type
    };    
  }
  
  template <typename T>
  T make_compartment(CompartmentInfo const compartment_info)
  {
    
    return 1;
  }

}

#endif // BLOFELD_COMPARTMENT_TYPES_H
