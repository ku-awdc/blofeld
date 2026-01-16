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
    balancing       // Fixed size of 1 and allows negative values (for birth/death) - n=0 or n=1
  };

  // Literal type
  struct CompType
  {
    CompCont const compcont;
    size_t const n;
    
    consteval auto is_active() const
      -> bool
    {
      return compcont!=CompCont::disabled && n!=0;
    }
    
  };
  
  // Helper function
  consteval CompType component(size_t const n)
  {
    if (n==0) {
      CompType ct = {
        .compcont = CompCont::disabled,
        .n = 1
      };    
      return ct;
    } else {
      CompType ct = {
        .compcont = CompCont::array,
        .n = n
      };    
      return ct;
    }
  }

}

#endif // BLOFELD_MODEL_COMP_TYPES_H
