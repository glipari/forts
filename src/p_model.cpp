#include "model.hpp"

using namespace std;

/** Model with parameters. */
Parameter::Parameter(string n, int mi, int ma) : name(n), min(mi), max(ma) {}

//Parameter::Parameter() {}

void Model::add_param(const Parameter &param)
{
    parameters.push_back(param);
}

const std::vector<Parameter>& Model::get_parameters() const
{
    return parameters;
}
