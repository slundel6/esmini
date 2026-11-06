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

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "pugixml.hpp"

class SE_Rand;

namespace scenarioengine
{
    class OSCParameterDistribution
    {
        struct ParameterValueEntry
        {
            std::string name;
            std::string value;
        };

    public:
        struct DistributionRange
        {
            bool   is_set      = false;
            double lower_limit = 0.0;
            double upper_limit = 0.0;
        };

        struct ProbabilityDistributionSetElement
        {
            std::string value;
            double      weight = 0.0;
        };

        struct HistogramBin
        {
            DistributionRange range;
            double            weight = 0.0;
        };

        enum class StochasticDistributionType
        {
            UNDEFINED,
            PROBABILITY_DISTRIBUTION_SET,
            NORMAL,
            LOG_NORMAL,
            UNIFORM,
            POISSON,
            HISTOGRAM,
            USER_DEFINED
        };

        struct StochasticDistribution
        {
            std::string                                    parameter_name;
            StochasticDistributionType                     type           = StochasticDistributionType::UNDEFINED;
            double                                         expected_value = 0.0;  // Normal, LogNormal, Poisson
            double                                         variance       = 0.0;  // Normal, LogNormal
            DistributionRange                              range;                 // optional for Normal, LogNormal, Poisson; required for Uniform
            std::vector<ProbabilityDistributionSetElement> elements;              // ProbabilityDistributionSet
            std::vector<HistogramBin>                      bins;                  // Histogram
            std::string                                    user_defined_type;     // UserDefinedDistribution
            std::string                                    user_defined_content;  // UserDefinedDistribution
        };

        struct Stochastic
        {
            unsigned int                        number_of_test_runs = 0;
            bool                                random_seed_set     = false;  // randomSeed supplied in XML
            unsigned int                        random_seed         = 0;
            std::vector<StochasticDistribution> distributions;
        };

    private:
        int ParseStochastic(pugi::xml_node stochastic);
        int ParseStochasticDistribution(pugi::xml_node node, StochasticDistribution& dist);
        int ParseRange(pugi::xml_node node, DistributionRange& range, bool required);

        // Stochastic sampling. Samples for a given run index are deterministic (seeded by randomSeed + index),
        // so any run can be reproduced in isolation, e.g. via --param_permutation
        const std::vector<ParameterValueEntry>& GetStochasticSample();
        std::string                             SampleDistribution(const StochasticDistribution& dist, SE_Rand& rand);

        std::vector<std::vector<std::vector<ParameterValueEntry>>> param_list_;
        Stochastic                                                 stochastic_;
        std::vector<ParameterValueEntry>                           stochastic_sample_;             // values for current run
        int                                                        stochastic_sample_index_ = -1;  // run index of stochastic_sample_
        std::string                                                filename_;
        std::string                                                scenario_filename_;
        int                                                        index_;
        int                                                        requested_index_;
        pugi::xml_document                                         doc_;

    public:
        OSCParameterDistribution()
        {
            Reset();
        }
        ~OSCParameterDistribution();
        static OSCParameterDistribution& Inst();

        int          Load(std::string filename);
        int          Load(pugi::xml_document& doc);
        int          Parse();
        unsigned int GetNumPermutations();
        unsigned int GetNumParameters();
        void         Reset();
        std::string  GetFilename() const
        {
            return filename_;
        }
        std::string GetScenarioFileName() const;
        // Returns current permutation index
        int GetIndex() const
        {
            return index_;
        }
        int SetIndex(unsigned int index);
        int SetRequestedIndex(unsigned int index);
        int GetRequestedIndex() const
        {
            return requested_index_;
        }
        int                 IncrementIndex();
        ParameterValueEntry GetParameterEntry(unsigned int param_index);
        std::string         GetParamName(unsigned int param_index);
        std::string         GetParamValue(unsigned int param_index);
        std::string         AddInfoToFilename(std::string filename);
        std::string         AddInfoToFilepath(std::string filepath);
        const Stochastic&   GetStochastic() const
        {
            return stochastic_;
        }
        void SetStochasticSeed(unsigned int seed);
        bool IsParamDist     = true;
        bool isDeterministic = true;
    };
}  // namespace scenarioengine
