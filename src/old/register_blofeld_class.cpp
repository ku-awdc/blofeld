#include <Rcpp.h>

namespace blofeld
{
  enum class ClassType{
    Population,
    Spread,
    Detect
  };

  struct ClassStatus
  {
    int number;
    ClassType type;
    bool active;
    ClassStatus() = delete;
    ClassStatus(int number_, ClassType type_)
      : number(number_), type(type_), active(true) {}
  };

  class ClassRegister
  {
  private:
    std::vector<ClassStatus> m_status;

  public:
    int register_class(ClassType type)
    {
      const int number = m_status.size()+1;
      m_status.emplace_back(number, type);
      Rcpp::Rcout << "Registering #" << number << "\n";
      return number;
    }

    void deregister_class(int number)
    {
      if(number <= 0 || number > m_status.size()) Rcpp::stop("Attempt to deregister invalid class instance number");
      if(!m_status[number-1].active) Rcpp::stop("Attempt to deregister inactive class instance number");

      Rcpp::Rcout << "Deregistering #" << number << "\n";
      m_status[number-1].active = false;
    }

    std::vector<ClassStatus> const& get_status() const
    {
      return m_status;
    }

    Rcpp::DataFrame get_status_R() const
    {
      const int n = m_status.size();
      Rcpp::IntegerVector number(n);
      Rcpp::StringVector type(n);
      Rcpp::LogicalVector active(n);
      for(int i=0; i<n; ++i)
      {
        ClassStatus const& stat = m_status[i];
        number[i] = stat.number;
        const std::string tpn = [](ClassType tp) -> std::string {
          using namespace std::literals::string_literals;
          if(tp == ClassType::Population){
            return "Population"s;
          }else if(tp == ClassType::Spread){
            return "Spread"s;
          }else if(tp == ClassType::Detect){
            return "Detect"s;
          }else{
            Rcpp::stop("Unrecognised ClassType");
          }
        }(stat.type);
        type[i] = tpn;
        active[i] = static_cast<bool>(stat.active);
      }

      Rcpp::DataFrame rv = Rcpp::DataFrame::create(Rcpp::_["Number"] = number);
      rv.push_back(type, "Type");
      rv.push_back(active, "Active");

      return rv;
    }

  };

}

static blofeld::ClassRegister class_register;

// [[Rcpp::interfaces(cpp)]]
int register_blofeld_class(std::string const& type)
{
  const blofeld::ClassType tp = [](std::string const& tp) -> blofeld::ClassType {
    using namespace std::literals::string_literals;
    if(tp == "Population"s){
      return blofeld::ClassType::Population;
    }else if(tp == "Spread"s){
      return blofeld::ClassType::Spread;
    }else if(tp == "Detect"s){
      return blofeld::ClassType::Detect;
    }else{
      Rcpp::stop("Unrecognised ClassType");
    }
  }(type);
  const int number = class_register.register_class(tp);
  return number;
}

// [[Rcpp::interfaces(cpp)]]
void deregister_blofeld_class(int number)
{
  class_register.deregister_class(number);
}

Rcpp::DataFrame registered_blofeld_classes()
{
  Rcpp::DataFrame rv = class_register.get_status_R();
  return rv;
}
