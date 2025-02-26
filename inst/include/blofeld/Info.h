#ifndef BLOFELD_INFO_H
#define BLOFELD_INFO_H

namespace blofeld
{
  class Info
  {
    /*
    Class to contain:
    0. ABI version
    1. Current date/time
    2. Info index number (info can be created from scratch or copied within pop etc, so copy/move etc will be needed)
    3. Tracking of populations/spreads/detects already added and giving them an index number
    4. Possibly global fixed data
    */
  private:
    
    // TODO: class in which we embed copies so it is only that we need to override constructors for, and not everything...!
    int m_copies = 0;
    
    // TODO: will static here cause problems with mixing between packages?
    static constexpr int s_ABI_version = 0;
    
  public:
    Info()
    {
      Rcpp::Rcout << "Constructor\n";
      m_copies++;
    }
    
    Info(const Info& from)
    {
      Rcpp::Rcout << "Copy\n";
    }
    
    Info& operator=(const Info& from)
    {
      if (this != &from){
        Rcpp::Rcout << "Copy assignment\n";
      }
      return *this;
    }

    Info(Info&& from) noexcept
    {
      Rcpp::Rcout << "Move\n";
    }
    
    Info& operator=(Info&& from)
    {
      if (this != &from){
        Rcpp::Rcout << "Move assignment\n";
      }
      return *this;
    }
    
    static constexpr int get_ABI_version()
    {
      return s_ABI_version;
    }
    
    int get_copies() const
    {
      return m_copies;
    }
    
    ~Info()
    {
      Rcpp::Rcout << "Removing\n";
    }
  };
  
} //blofeld

#endif //BLOFELD_INFO_H
