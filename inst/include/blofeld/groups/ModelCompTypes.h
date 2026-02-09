#ifndef BLOFELD_MODEL_COMP_TYPES_H
#define BLOFELD_MODEL_COMP_TYPES_H

namespace blofeld
{

  enum class ModelType
  {
    deterministic,  // Uses double and simple maths
    stochastic      // Uses int and random sampling
  };

  enum class CompCont
  {
    disabled,       // Removed - compiles to nothing: n=0
    array,          // Fixed size (including 1, but not zero): n>0
    inplace_vector, // Emulation of c++26 inplace_vector i.e. stack-based, dynamic up to max size: n>0 (=max)
    vector,         // Heap-based, dynamic - s_ctype.n is ignored: n=1
    birth_death     // Fixed size of 1 and allows negative values (for birth/death) - n=0 or n=1
  };

  enum class CompCarry
  {
    sequential,       // Traditional carry-through over sequential time points
    immediate,        // Carry-through within a single time point
    none              // For n==0 or n==1
  };

  // Literal type
  struct CompType
  {
    size_t const n;
    CompCont const compcont;
    CompCarry const compcarry;
    
    consteval auto is_active() const
      -> bool
    {
      return compcont!=CompCont::disabled && n!=0;
    }
    
  };
  
  // Helper function (with default arguments - note that we use int not size_t for n)
  consteval CompType component(size_t const n = 1, CompCont const cont_type = CompCont::array, CompCarry const carry_type = CompCarry::sequential)
  {    
    // Allow shortcut of specifying n==0 with default cont_type==array to mean disable:
    if (n == 0 && (cont_type == CompCont::array || cont_type == CompCont::disabled)) {
      CompType ct = {
        .n = 0,
        .compcont = CompCont::disabled,
        .compcarry = CompCarry::none
      };    
      return ct;
    }
    
    // Checks:
    if (cont_type == CompCont::disabled) {
      // if (n != 0)
      throw("For CompCont::disabled n must be equal to 0");
    }
    if (cont_type == CompCont::birth_death) {
      if (n != 1) throw("For CompCont::birth_death n must be equal to 1");
    }
    if (n==0) {
      // Note: with n=1 inplace_vector and array are almost identical, but allow both for bug hunting etc
      if (cont_type != CompCont::vector) throw("For CompCont::array and CompCont::inplace_vector n must be greater than 0");
    }
    if (n < 0) {
      throw("Invalid n < 0");
    }
        
    // For array or inplace_vector or birth_death with n==1 we can ignore carry_type:
    if (n == 1 && cont_type != CompCont::vector) {
      CompType ct = {
        .n = n,
        .compcont = cont_type,
        .compcarry = CompCarry::none
      };    
      return ct;
    }
    
    // Otherwise we can use what we were given:
    CompType ct = {
      .n = n,
      .compcont = cont_type,
      .compcarry = carry_type
    };    
    return ct;
  }

}

#endif // BLOFELD_MODEL_COMP_TYPES_H
