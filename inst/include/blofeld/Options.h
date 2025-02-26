#ifndef BLOFELD_OPTIONS_H
#define BLOFELD_OPTIONS_H


namespace blofeld
{
  struct OptionsEmpty
  {
  };
  
  template<class T_CustomOpts = OptionsEmpty>
  struct Options
  {
    bool debug = true;
    int assert_level = 0;
    int log_level = 0;
    T_CustomOpts custom {};
  };
    
} //blofeld

#endif //BLOFELD_OPTIONS_H
