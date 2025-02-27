// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#ifndef RCPP_blofeld_RCPPEXPORTS_H_GEN_
#define RCPP_blofeld_RCPPEXPORTS_H_GEN_

#include <Rcpp.h>

namespace blofeld {

    using namespace Rcpp;

    namespace {
        void validateSignature(const char* sig) {
            Rcpp::Function require = Rcpp::Environment::base_env()["require"];
            require("blofeld", Rcpp::Named("quietly") = true);
            typedef int(*Ptr_validate)(const char*);
            static Ptr_validate p_validate = (Ptr_validate)
                R_GetCCallable("blofeld", "_blofeld_RcppExport_validate");
            if (!p_validate(sig)) {
                throw Rcpp::function_not_exported(
                    "C++ function with signature '" + std::string(sig) + "' not found in blofeld");
            }
        }
    }

}

#endif // RCPP_blofeld_RCPPEXPORTS_H_GEN_
