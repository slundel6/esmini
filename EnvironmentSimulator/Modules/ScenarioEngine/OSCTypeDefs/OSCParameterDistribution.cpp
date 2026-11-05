/*
 * esmini - Environment Simulator Minimalistic
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#include "CommonMini.hpp"
#include "OSCParameterDistribution.hpp"
#include "logger.hpp"
#include <iomanip>
#include <sstream>
#include <functional>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "Missing <filesystem> header"
#endif

using namespace scenarioengine;

OSCParameterDistribution& OSCParameterDistribution::Inst()
{
    static OSCParameterDistribution instance_;
    return instance_;
}

OSCParameterDistribution::~OSCParameterDistribution()
{
    Reset();
}
int OSCParameterDistribution::Load(pugi::xml_document& doc)
{
    doc_ = std::move(doc);

    return Parse();
}

int OSCParameterDistribution::Load(std::string filename)
{
    std::vector<std::string> file_name_candidates;

    file_name_candidates.push_back(filename);

    // Check registered paths
    for (size_t i = 0; i < SE_Env::Inst().GetPaths().size(); i++)
    {
        file_name_candidates.push_back(CombineDirectoryPathAndFilepath(SE_Env::Inst().GetPaths()[i], filename));
    }

    pugi::xml_parse_result result;
    size_t                 i;
    for (i = 0; i < file_name_candidates.size(); i++)
    {
        if (fs::exists(file_name_candidates[i].c_str()))
        {
            result = doc_.load_file(file_name_candidates[i].c_str());
            if (!result)
            {
                LOG_INFO("{}: {} at offset (character position): {}", file_name_candidates[i], result.description(), result.offset);
                return -1;
            }
            else
            {
                break;
            }
        }
    }

    if (i == file_name_candidates.size())
    {
        if (IsParamDist)
        {
            LOG_ERROR("Failed to load parameter distribution file {}. Tried:", filename);
            for (unsigned int j = 0; j < file_name_candidates.size(); j++)
            {
                LOG_INFO("   {}\n", file_name_candidates[j]);
            }
            LOG_WARN("continue without road description\n");
        }
        else
        {
            return 0;  // Failed to load file, Used for single run as osc. Taken care as only osc.
        }
    }
    else
    {
        filename_ = file_name_candidates[i];
        LOG_INFO("Loaded {}", filename_);
    }

    return Parse();
}

int OSCParameterDistribution::Parse()
{
    pugi::xml_node node = doc_.child("OpenSCENARIO");
    if (!node)
    {
        // Another try
        node = doc_.child("OpenScenario");
    }

    if (!node)
    {
        throw std::runtime_error("Couldn't find OpenSCENARIO or OpenScenario element in - check XML!");
    }

    node = node.child("ParameterValueDistribution");
    if (node.empty())
    {
        if (IsParamDist)
        {
            LOG_WARN("No or empty ParameterValueDistribution element");
        }
        else
        {
            return 0;  // no distribution defined in this case is ok(used for single run as osc)
        }
    }

    pugi::xml_node scenario_filename_node = node.child("ScenarioFile");
    if (scenario_filename_node.empty())
    {
        LOG_ERROR("ScenarioFile missing");
    }
    else
    {
        scenario_filename_ = scenario_filename_node.attribute("filepath").value();
        if (scenario_filename_.empty())
        {
            LOG_ERROR("Scenario filepath attribute missing");
        }
        else
        {
            scenario_filename_ = (fs::path(filename_).parent_path() / fs::path(scenario_filename_)).string();
        }
    }

    pugi::xml_node deterministic = node.child("Deterministic");
    if (!deterministic.empty())
    {
        for (pugi::xml_node child_node = deterministic.first_child(); child_node; child_node = child_node.next_sibling())
        {
            std::string child_node_name(child_node.name());

            if (child_node_name == "DeterministicMultiParameterDistribution")
            {
                pugi::xml_node value_set_dist = child_node.child("ValueSetDistribution");
                if (value_set_dist.empty())
                {
                    LOG_ERROR("ValueSetDistribution missing");
                    return -1;
                }

                std::vector<std::vector<ParameterValueEntry>> p_list;
                for (pugi::xml_node value_set = value_set_dist.child("ParameterValueSet"); value_set;
                     value_set                = value_set.next_sibling("ParameterValueSet"))
                {
                    std::vector<ParameterValueEntry> p_value_list;
                    for (pugi::xml_node param_assign = value_set.child("ParameterAssignment"); param_assign;
                         param_assign                = param_assign.next_sibling("ParameterAssignment"))
                    {
                        ParameterValueEntry entry = {param_assign.attribute("parameterRef").value(), param_assign.attribute("value").value()};
                        p_value_list.push_back(entry);
                    }
                    p_list.push_back(p_value_list);
                }
                param_list_.push_back(p_list);
            }
            else if (child_node_name == "DeterministicSingleParameterDistribution")
            {
                std::string param_name = child_node.attribute("parameterName").value();
                if (param_name.empty() || param_name == "")
                {
                    LOG_ERROR("Missing single distribution parameter name");
                    return -1;
                }

                pugi::xml_node dist = child_node.first_child();
                if (dist.empty())
                {
                    LOG_ERROR("Missing single distribution definition");
                    return -1;
                }

                if (!strcmp(dist.name(), "DistributionSet"))
                {
                    std::vector<std::vector<ParameterValueEntry>> p_list;
                    ParameterValueEntry                           entry;
                    entry.name = param_name;

                    for (pugi::xml_node elem = dist.child("Element"); elem; elem = elem.next_sibling("Element"))
                    {
                        entry.value                                   = elem.attribute("value").value();
                        std::vector<ParameterValueEntry> p_value_list = {entry};
                        p_list.push_back(p_value_list);
                    }
                    param_list_.push_back(p_list);
                }
                else if (!strcmp(dist.name(), "DistributionRange"))
                {
                    std::vector<std::vector<ParameterValueEntry>> p_list;
                    ParameterValueEntry                           entry;
                    entry.name = param_name;

                    double step_width = std::atof(dist.attribute("stepWidth").value());

                    pugi::xml_node range = dist.child("Range");
                    if (range.empty())
                    {
                        LOG_ERROR("Distribution range missing");
                        return -1;
                    }

                    double lower_limit = std::atof(range.attribute("lowerLimit").value());
                    double upper_limit = std::atof(range.attribute("upperLimit").value());

                    if (upper_limit < lower_limit)
                    {
                        LOG_ERROR("Distribution range invalid range {:.2f}..{:.2f}", lower_limit, upper_limit);
                        return -1;
                    }
                    for (double v = lower_limit; v < upper_limit + SMALL_NUMBER; v += step_width)
                    {
                        entry.value                                   = std::to_string(v);
                        std::vector<ParameterValueEntry> p_value_list = {entry};
                        p_list.push_back(p_value_list);
                    }
                    param_list_.push_back(p_list);
                }
                else if (!strcmp(dist.name(), "UserDefinedDistribution"))
                {
                    LOG_ERROR("UserDefinedDistribution not yet supported");
                    return -1;
                }
                else
                {
                    LOG_ERROR("Unexpected distribution definition");
                    return -1;
                }
            }
            else
            {
                LOG_ERROR("Unexpected deterministic distribution type {}", child_node_name);
            }
        }
    }

    pugi::xml_node stochastic = node.child("Stochastic");
    if (!stochastic.empty())
    {
        if (!deterministic.empty())
        {
            LOG_WARN("Found BOTH Deterministic and Stochastic distribution elements. Only one expected. Using Deterministic.");
        }
        else
        {
            if (ParseStochastic(stochastic) != 0)
            {
                return -1;
            }
            isDeterministic = false;
            if (!stochastic_.random_seed_set)
            {
                // no seed given, pick one and report it so the batch can be reproduced
                stochastic_.random_seed = static_cast<double>(SE_Rand().GetSeed());
                LOG_INFO("Stochastic distribution: no randomSeed specified, using {:.0f}", stochastic_.random_seed);
            }
        }
    }
    else if (deterministic.empty())
    {
        LOG_ERROR("No distribution defined. Expected Determinstic or Stochastic");
    }

    return 0;
}

int OSCParameterDistribution::ParseRange(pugi::xml_node node, DistributionRange& range, bool required)
{
    range.is_set = false;

    if (node.empty())
    {
        if (required)
        {
            LOG_ERROR("Distribution range missing");
            return -1;
        }
        return 0;
    }

    if (node.attribute("lowerLimit").empty() || node.attribute("upperLimit").empty())
    {
        LOG_ERROR("Range missing lowerLimit and/or upperLimit attribute");
        return -1;
    }

    range.lower_limit = std::atof(node.attribute("lowerLimit").value());
    range.upper_limit = std::atof(node.attribute("upperLimit").value());

    if (range.upper_limit < range.lower_limit)
    {
        LOG_ERROR("Distribution range invalid range {:.2f}..{:.2f}", range.lower_limit, range.upper_limit);
        return -1;
    }

    range.is_set = true;

    return 0;
}

int OSCParameterDistribution::ParseStochasticDistribution(pugi::xml_node node, StochasticDistribution& dist)
{
    dist.parameter_name = node.attribute("parameterName").value();
    if (dist.parameter_name.empty())
    {
        LOG_ERROR("Missing stochastic distribution parameter name");
        return -1;
    }

    pugi::xml_node dist_node = node.first_child();
    if (dist_node.empty())
    {
        LOG_ERROR("Missing stochastic distribution definition for parameter {}", dist.parameter_name);
        return -1;
    }

    std::string dist_name(dist_node.name());

    if (dist_name == "ProbabilityDistributionSet")
    {
        dist.type = StochasticDistributionType::PROBABILITY_DISTRIBUTION_SET;
        for (pugi::xml_node elem = dist_node.child("Element"); elem; elem = elem.next_sibling("Element"))
        {
            if (elem.attribute("value").empty() || elem.attribute("weight").empty())
            {
                LOG_ERROR("ProbabilityDistributionSet Element missing value and/or weight attribute");
                return -1;
            }

            ProbabilityDistributionSetElement element;
            element.value  = elem.attribute("value").value();
            element.weight = elem.attribute("weight").as_double();
            if (element.weight < 0.0)
            {
                LOG_ERROR("ProbabilityDistributionSet Element negative weight {:.2f}", element.weight);
                return -1;
            }
            dist.elements.push_back(element);
        }

        if (dist.elements.empty())
        {
            LOG_ERROR("ProbabilityDistributionSet requires at least one Element");
            return -1;
        }
    }
    else if (dist_name == "NormalDistribution" || dist_name == "LogNormalDistribution")
    {
        dist.type = dist_name == "NormalDistribution" ? StochasticDistributionType::NORMAL : StochasticDistributionType::LOG_NORMAL;

        if (dist_node.attribute("expectedValue").empty() || dist_node.attribute("variance").empty())
        {
            LOG_ERROR("{} missing expectedValue and/or variance attribute", dist_name);
            return -1;
        }

        dist.expected_value = dist_node.attribute("expectedValue").as_double();
        dist.variance       = dist_node.attribute("variance").as_double();

        if (dist.variance < 0.0)
        {
            LOG_ERROR("{} negative variance {:.2f}", dist_name, dist.variance);
            return -1;
        }

        if (ParseRange(dist_node.child("Range"), dist.range, false) != 0)
        {
            return -1;
        }
    }
    else if (dist_name == "UniformDistribution")
    {
        dist.type = StochasticDistributionType::UNIFORM;
        if (ParseRange(dist_node.child("Range"), dist.range, true) != 0)
        {
            return -1;
        }
    }
    else if (dist_name == "PoissonDistribution")
    {
        dist.type = StochasticDistributionType::POISSON;

        if (dist_node.attribute("expectedValue").empty())
        {
            LOG_ERROR("PoissonDistribution missing expectedValue attribute");
            return -1;
        }

        dist.expected_value = std::atof(dist_node.attribute("expectedValue").value());

        if (ParseRange(dist_node.child("Range"), dist.range, false) != 0)
        {
            return -1;
        }
    }
    else if (dist_name == "Histogram")
    {
        dist.type = StochasticDistributionType::HISTOGRAM;
        for (pugi::xml_node bin_node = dist_node.child("Bin"); bin_node; bin_node = bin_node.next_sibling("Bin"))
        {
            if (bin_node.attribute("weight").empty())
            {
                LOG_ERROR("Histogram Bin missing weight attribute");
                return -1;
            }

            HistogramBin bin;
            bin.weight = bin_node.attribute("weight").as_double();
            if (bin.weight < 0.0)
            {
                LOG_ERROR("Histogram Bin negative weight {:.2f}", bin.weight);
                return -1;
            }

            if (ParseRange(bin_node.child("Range"), bin.range, true) != 0)
            {
                return -1;
            }
            dist.bins.push_back(bin);
        }

        if (dist.bins.empty())
        {
            LOG_ERROR("Histogram requires at least one Bin");
            return -1;
        }
    }
    else if (dist_name == "UserDefinedDistribution")
    {
        dist.type                 = StochasticDistributionType::USER_DEFINED;
        dist.user_defined_type    = dist_node.attribute("type").value();
        dist.user_defined_content = dist_node.text().get();

        if (dist.user_defined_type.empty())
        {
            LOG_ERROR("UserDefinedDistribution missing type attribute");
            return -1;
        }
    }
    else
    {
        LOG_ERROR("Unexpected stochastic distribution type {}", dist_name);
        return -1;
    }

    return 0;
}

int OSCParameterDistribution::ParseStochastic(pugi::xml_node stochastic)
{
    if (stochastic.attribute("numberOfTestRuns").empty())
    {
        LOG_ERROR("Stochastic missing numberOfTestRuns attribute");
        return -1;
    }

    int n_runs = stochastic.attribute("numberOfTestRuns").as_int();
    if (n_runs < 0)
    {
        LOG_ERROR("Stochastic numberOfTestRuns invalid value {}", n_runs);
        return -1;
    }
    stochastic_.number_of_test_runs = static_cast<unsigned int>(n_runs);

    if (!stochastic.attribute("randomSeed").empty())
    {
        stochastic_.random_seed     = stochastic.attribute("randomSeed").as_double();
        stochastic_.random_seed_set = true;
    }

    for (pugi::xml_node child_node = stochastic.first_child(); child_node; child_node = child_node.next_sibling())
    {
        std::string child_node_name(child_node.name());

        if (child_node_name == "StochasticDistribution")
        {
            StochasticDistribution dist;
            if (ParseStochasticDistribution(child_node, dist) != 0)
            {
                return -1;
            }
            stochastic_.distributions.push_back(dist);
        }
        else
        {
            LOG_ERROR("Unexpected stochastic element {}", child_node_name);
        }
    }

    if (stochastic_.distributions.empty())
    {
        LOG_ERROR("Stochastic requires at least one StochasticDistribution");
        return -1;
    }

    return 0;
}

unsigned int OSCParameterDistribution::GetNumPermutations()
{
    unsigned int n = 1;

    if (param_list_.size() == 0 && isDeterministic == true)
    {
        return 0;
    }

    if (isDeterministic)
    {
        for (size_t i = 0; i < param_list_.size(); i++)
        {
            n *= static_cast<unsigned int>(param_list_[i].size());
        }
    }
    else
    {
        n = GetStochastic().number_of_test_runs;
    }

    return n;
}

unsigned int OSCParameterDistribution::GetNumParameters()
{
    if (index_ < 0)
    {
        return 0;
    }

    if (!isDeterministic)
    {
        return static_cast<unsigned int>(GetStochasticSample().size());
    }

    // Calculate number of parameters in current permutation
    unsigned int num_parameters = 0;

    unsigned int n = static_cast<unsigned int>(index_);

    for (unsigned int i = static_cast<unsigned int>(param_list_.size()); i > 0; i--)
    {
        unsigned int q = n / static_cast<unsigned int>(param_list_[i - 1].size());
        unsigned int r = n % static_cast<unsigned int>(param_list_[i - 1].size());

        // add parameters from this set
        num_parameters += static_cast<unsigned int>(param_list_[i - 1][r].size());

        n = q;
    }

    return num_parameters;
}

OSCParameterDistribution::ParameterValueEntry OSCParameterDistribution::GetParameterEntry(unsigned int param_index)
{
    unsigned int n_parameters = GetNumParameters();

    if (param_index >= n_parameters)
    {
        return {"", ""};
    }
    else if (index_ >= static_cast<int>(GetNumPermutations()))
    {
        return {"", ""};
    }
    else if (GetNumPermutations() == 0 || index_ < 0)
    {
        return {"", ""};
    }

    if (!isDeterministic)
    {
        return GetStochasticSample()[param_index];
    }

    // mirror internal index starting from end, since permutations are traversed that way
    // varying the latest variable first
    unsigned int p_idx         = n_parameters - param_index - 1;
    unsigned int param_counter = 0;
    unsigned int n             = static_cast<unsigned int>(index_);

    for (unsigned int i = static_cast<unsigned int>(param_list_.size()); i > 0; i--)
    {
        unsigned int q = n / static_cast<unsigned int>(param_list_[i - 1].size());
        unsigned int r = n % static_cast<unsigned int>(param_list_[i - 1].size());

        // add parameters from this set
        param_counter += static_cast<unsigned int>(param_list_[i - 1][r].size());

        if (p_idx < param_counter)
        {
            return param_list_[i - 1][r][param_counter - p_idx - 1];
        }

        n = q;
    }

    return {"", ""};
}

const std::vector<OSCParameterDistribution::ParameterValueEntry>& OSCParameterDistribution::GetStochasticSample()
{
    if (index_ < 0 || stochastic_sample_index_ == index_)
    {
        return stochastic_sample_;
    }

    // Seed per run index so that each run is reproducible independently of execution order
    int64_t       seed = static_cast<int64_t>(stochastic_.random_seed);
    std::seed_seq seq{static_cast<uint32_t>(seed & 0xffffffff), static_cast<uint32_t>((seed >> 32) & 0xffffffff), static_cast<uint32_t>(index_)};
    std::vector<uint32_t> run_seed(1);
    seq.generate(run_seed.begin(), run_seed.end());

    // Dedicated generator, not SE_Env::Inst().GetRand(), to not interfere with the global --seed controlled sequence
    SE_Rand rand;
    rand.SetSeed(run_seed[0]);

    stochastic_sample_.clear();
    for (const auto& dist : stochastic_.distributions)
    {
        stochastic_sample_.push_back({dist.parameter_name, SampleDistribution(dist, rand)});
    }
    stochastic_sample_index_ = index_;

    return stochastic_sample_;
}

std::string OSCParameterDistribution::SampleDistribution(const StochasticDistribution& dist, SE_Rand& rand)
{
    std::mt19937& gen = rand.GetGenerator();

    // Draw from a continuous distribution, rejecting samples outside optional range (truncated distribution)
    // There's no built in "sample between these 2 values" in C++, so we resample until we get value inside the defined range. If we are not inside
    // range at max_attempts, we clamp to nearest range limit
    auto sample_truncated = [&](const std::function<double()>& draw) -> double
    {
        const int max_attempts = 1000;
        double    v            = draw();
        // Allowed range is lowerLimit <= value <= upperLimit, below we check if sample is outside that
        for (int i = 0; dist.range.is_set && (v < dist.range.lower_limit || v > dist.range.upper_limit); i++)
        {
            if (i >= max_attempts)
            {
                LOG_WARN("{}: failed to sample within range {:.2f}..{:.2f}, clamping",
                         dist.parameter_name,
                         dist.range.lower_limit,
                         dist.range.upper_limit);
                return CLAMP(v, dist.range.lower_limit, dist.range.upper_limit);
            }
            v = draw();
        }
        return v;
    };

    switch (dist.type)
    {
        case StochasticDistributionType::PROBABILITY_DISTRIBUTION_SET:
        {
            // Get the weights of each bin specified in the param dist
            std::vector<double> weights;
            for (const auto& e : dist.elements)
            {
                weights.push_back(e.weight);
            }
            // Create a distribution to sample from based on the weights (P(i) = weight_i / sum(weights))
            std::discrete_distribution<size_t> d(weights.begin(), weights.end());
            // Get a value based on a randomly sampled index in discrete distribution
            return dist.elements[d(gen)].value;
        }
        case StochasticDistributionType::NORMAL:
        {
            // Create a normal distribution and sample from it until we hit a value inside the range
            // The resulting truncated distribution will be a rescaled normal distribution
            if (dist.variance < SMALL_NUMBER)
            {
                // Degenerate distribution, std::normal_distribution requires expectedValue > 0
                return std::to_string(dist.expected_value);
            }
            std::normal_distribution<double> d(dist.expected_value, sqrt(dist.variance));
            return std::to_string(sample_truncated([&]() { return d(gen); }));
        }
        case StochasticDistributionType::LOG_NORMAL:
        {
            // expectedValue and variance refer to the log-normal distribution itself, convert to underlying normal parameters
            if (dist.expected_value < SMALL_NUMBER)
            {
                LOG_ERROR("{}: LogNormalDistribution requires expectedValue > 0", dist.parameter_name);
                return "";
            }
            if (dist.variance < SMALL_NUMBER)
            {
                // Degenerate distribution, std::lognormal_distribution requires expectedValue > 0
                return std::to_string(dist.expected_value);
            }
            double                              sigma2 = log(1.0 + dist.variance / (dist.expected_value * dist.expected_value));
            double                              mu     = log(dist.expected_value) - 0.5 * sigma2;
            std::lognormal_distribution<double> d(mu, sqrt(sigma2));
            return std::to_string(sample_truncated([&]() { return d(gen); }));
        }
        case StochasticDistributionType::UNIFORM:
        {
            return std::to_string(rand.GetRealBetween(dist.range.lower_limit, dist.range.upper_limit));
        }
        case StochasticDistributionType::POISSON:
        {
            std::poisson_distribution<int> d(dist.expected_value);
            return std::to_string(static_cast<int>(sample_truncated([&]() { return static_cast<double>(d(gen)); })));
        }
        case StochasticDistributionType::HISTOGRAM:
        {
            // Get the weights of each bin specified in the param dist
            std::vector<double> weights;
            for (const auto& b : dist.bins)
            {
                weights.push_back(b.weight);
            }
            // Create a distribution to sample from based on the weights (P(i) = weight_i / sum(weights))
            std::discrete_distribution<size_t> bin_d(weights.begin(), weights.end());
            // Get a histogram bin based on a randomly sampled index in discrete distribution
            const HistogramBin& bin = dist.bins[bin_d(gen)];
            // Pick uniformly inside the selected bin and return the value
            return std::to_string(rand.GetRealBetween(bin.range.lower_limit, bin.range.upper_limit));
        }
        case StochasticDistributionType::USER_DEFINED:
            LOG_ERROR("{}: UserDefinedDistribution (type {}) not supported", dist.parameter_name, dist.user_defined_type);
            return "";
        default:
            LOG_ERROR("{}: undefined stochastic distribution", dist.parameter_name);
            return "";
    }
}

std::string OSCParameterDistribution::GetParamName(unsigned int param_index)
{
    return GetParameterEntry(param_index).name;
}

std::string OSCParameterDistribution::GetParamValue(unsigned int param_index)
{
    return GetParameterEntry(param_index).value;
}

std::string OSCParameterDistribution::AddInfoToFilename(std::string filename)
{
    std::string base_name = FileNameWithoutExtOf(filename);
    std::string ext       = FileNameExtOf(filename);

#if 1  // no leading zeros
    return FileNameWithoutExtOf(base_name) + "_" + std::to_string(GetIndex() + 1) + "_of_" + std::to_string(GetNumPermutations()) + ext;
#else  // leading zeros
    int number     = GetNumPermutations();
    int num_digits = 0;
    while (number != 0)
    {
        number /= 10;
        num_digits++;
    }
    std::ostringstream str;
    str << FileNameWithoutExtOf(base_name) << "_" << std::setw(num_digits) << std::setfill('0') << GetIndex() + 1 << "_of_"
        << std::to_string(GetNumPermutations()) << ext;
    return str.str();
#endif
}

std::string OSCParameterDistribution::AddInfoToFilepath(std::string filepath)
{
    std::string base_path = FilePathWithoutExtOf(filepath);
    std::string ext       = FileNameExtOf(filepath);

#if 1  // no leading zeros
    return base_path + "_" + std::to_string(GetIndex() + 1) + "_of_" + std::to_string(GetNumPermutations()) + ext;
#else  // leading zeros
    int number     = GetNumPermutations();
    int num_digits = 0;
    while (number != 0)
    {
        number /= 10;
        num_digits++;
    }
    std::ostringstream str;
    str << FileNameWithoutExtOf(base_name) << "_" << std::setw(num_digits) << std::setfill('0') << GetIndex() + 1 << "_of_"
        << std::to_string(GetNumPermutations()) << ext;
    return str.str();
#endif
}

void OSCParameterDistribution::Reset()
{
    for (size_t i = 0; i < param_list_.size(); i++)
    {
        for (size_t j = 0; j < param_list_[i].size(); j++)
        {
            param_list_[i][j].clear();
        }
        param_list_[i].clear();
    }
    param_list_.clear();
    stochastic_ = Stochastic();
    stochastic_sample_.clear();
    stochastic_sample_index_ = -1;
    isDeterministic          = true;
    filename_.clear();
    scenario_filename_.clear();
    index_           = -1;
    requested_index_ = 0;  // first permutation
}

std::string OSCParameterDistribution::GetScenarioFileName() const
{
    return scenario_filename_;
}

int OSCParameterDistribution::SetIndex(unsigned int index)
{
    if (index >= GetNumPermutations())
    {
        LOG_ERROR("Permutation index {} out of range (0..{})", index, GetNumPermutations() - 1);
        return -1;
    }

    index_           = static_cast<int>(index);
    requested_index_ = -1;  // set directly, no request

    return 0;
}

int OSCParameterDistribution::IncrementIndex()
{
    if (index_ < -1 || index_ >= static_cast<int>(GetNumPermutations() - 1))
    {
        LOG_ERROR("Can't increment index {}, would end up out of range (0..{})\n", index_, GetNumPermutations() - 1);
        return -1;
    }

    index_++;
    requested_index_ = -1;  // indicate calculated, not requested

    return index_;
}

int OSCParameterDistribution::SetRequestedIndex(unsigned int index)
{
    if (index >= GetNumPermutations())
    {
        LOG_ERROR("requested permutation index {} out of range (0..{})", index, GetNumPermutations() - 1);
        return -1;
    }

    requested_index_ = static_cast<int>(index);

    return 0;
}
