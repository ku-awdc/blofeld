#include <Rcpp.h>

class Base {
  public:
    Rcpp::NumericVector & data;
  Base(Rcpp::NumericVector & x) : data(x) {}
  void append(double x) {
    data.push_back(x);
  }
};
RCPP_EXPOSED_CLASS(Base)


class Container {

  public:
    Rcpp::List objects;
  Container(Rcpp::List objects) {
    this->objects = objects;
  }
  void append(double x) {
    for (int i = 0; i < objects.length(); i++) {
      Rcpp::NumericVector objvec = objects[i];
      Base obj( objvec );
      obj.append(x);
      objects[i] = objvec;
    }
  }
  Rcpp::List output() {
    return objects;
  }
};
RCPP_EXPOSED_CLASS(Container)

RCPP_MODULE(Container){
  using namespace Rcpp;
  class_<Container>("Container")
  .constructor< List >()
  .method( "append", &Container::append )
  .method( "output", &Container::output )
  ;
}
