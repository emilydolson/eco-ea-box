#include "config/ArgManager.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "Evo/World.h"
#include "Evo/Resource.h"


EMP_BUILD_CONFIG( BoxConfig,
  GROUP(DEFAULT, "Default settings for box experiment"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 5000, "Number of organisms in the popoulation."),
  VALUE(UPDATES, uint32_t, 10000, "How many generations should we process?"),
  VALUE(SELECTION, std::string, "TOURNAMENT", "What selection scheme should we use?"),
  VALUE(N_NEUTRAL, int, 0, "Number of neutral fitness functions"),
  VALUE(N_GOOD, int, 7, "Number of good fitness functions"),
  VALUE(N_BAD, int, 3, "Number of bad fitness functions"),
  VALUE(DISTANCE_CUTOFF, double, .1, "How close to origin does fitness gradient start"),
  VALUE(RESOURCE_INFLOW, double, 100, "How much resource enters the world each update"),
  VALUE(MUTATION_SIZE, double, .05, "Standard deviation of normal distribution mutations are seelcted from"),
  VALUE(PROBLEM_DIMENSIONS, int, 10, "How many axes does the box have?"),
  VALUE(RECOMBINATION, int, 0, "Does recombination happen?"),
  VALUE(TOURNAMENT_SIZE, int, 5, "Tournament size"),
  VALUE(COST, double, 0, "Cost of doing task unsuccessfully"),
  VALUE(FRAC, double, .0025, "Percent of resource individual can use"),
  VALUE(MAX_RES_USE, double, 5, "Maximum quantity of resource that individual can use")
)

using ORG_TYPE = emp::vector<double>;

class BoxWorld  : public emp::World<ORG_TYPE>{
public:

    BoxConfig config;
    uint32_t POP_SIZE;
    uint32_t UPDATES;
    int N_NEUTRAL;
    int N_GOOD;
    int N_BAD;
    int PROBLEM_DIMENSIONS;
    double DISTANCE_CUTOFF;
    double RESOURCE_INFLOW;
    double MUTATION_SIZE;
    std::string SELECTION;
    bool RECOMBINATION;
    int TOURNAMENT_SIZE;
    int GENOME_SIZE;
    double COST;
    double FRAC;
    double MAX_RES_USE;

    emp::vector<emp::Resource> resources;
    emp::vector< std::function<double(const ORG_TYPE &)> > fit_set;

    void SetupFitnessFunctions() {

        fit_set.resize(0);

        // Good hints
        for (int i = 0; i < N_GOOD; i++) {
            fit_set.push_back([i](const ORG_TYPE & org){
                double score = 1 - org[i];
                if (score < .8) {
                    return 0.0;
                }
                return score;
            });
        }

        // Bad hints
        for (int i = N_GOOD; i < N_GOOD + N_BAD; i++) {
            fit_set.push_back([i](const ORG_TYPE & org){
                double score = org[i];
                if (score < .8) {
                    return 0.0;
                }
                return score;
            });
        }

        // Neutral hints (these axes aren't evaluated)
        for (int i = PROBLEM_DIMENSIONS; i < GENOME_SIZE; i++) {
            fit_set.push_back([i](const ORG_TYPE & org){return org[i];});
        }

        std::function<double(const ORG_TYPE&)> goal_function = [this](const ORG_TYPE & org){
          double dist = 0;
          for (int i = 0; i < PROBLEM_DIMENSIONS; i++) {
              dist += emp::Pow(org[i], 2.0);
          }

          dist = sqrt(dist);
          if (dist > DISTANCE_CUTOFF) {
              return 0.01;
          }

          return 1.0/dist;
        };

        SetFitFun(goal_function);

        if (SELECTION == "LEXICASE") {
            fit_set.push_back(goal_function);
        } else if (SELECTION == "TOURNAMENT") {
            SetCache(true);
        }

    }


    void InitConfigs() {
        POP_SIZE = config.POP_SIZE();
        UPDATES = config.UPDATES();
        N_NEUTRAL = config.N_NEUTRAL();
        N_GOOD = config.N_GOOD();
        N_BAD = config.N_BAD();
        PROBLEM_DIMENSIONS = config.PROBLEM_DIMENSIONS();
        DISTANCE_CUTOFF = config.DISTANCE_CUTOFF();
        RESOURCE_INFLOW = config.RESOURCE_INFLOW();
        MUTATION_SIZE = config.MUTATION_SIZE();
        SELECTION = config.SELECTION();
        RECOMBINATION = config.RECOMBINATION();
        TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
        GENOME_SIZE = PROBLEM_DIMENSIONS + N_NEUTRAL;
        COST = config.COST();
        FRAC = config.FRAC();
        MAX_RES_USE = config.MAX_RES_USE();
    }

    void InitPop() {
        emp::Random & random = GetRandom();
        for (size_t i = 0; i < POP_SIZE; i++) {
          ORG_TYPE org(GENOME_SIZE);
          for (int i = 0; i < GENOME_SIZE; i++) {
              org[i] = random.GetDouble(1);
          }
          Inject(org);
        }
    }

    void Setup() {

        SetWellMixed(true);
        SystematicsOff();

        InitConfigs();

        resources.resize(0);
        for (int i=0; i<GENOME_SIZE; i++) {
            resources.push_back(emp::Resource(RESOURCE_INFLOW, RESOURCE_INFLOW, .01));
        }

        InitPop();

        // Setup the mutation function.
        SetMutFun( [this](ORG_TYPE & org, emp::Random & random) {
          //   uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
            for (uint32_t pos = 0; pos < GENOME_SIZE; pos++) {
              org[pos] += random.GetRandNormal(0, MUTATION_SIZE);
              if (org[pos] < 0) {
                  org[pos] = 0;
              } else if (org[pos] > 1) {
                  org[pos] = 1;
              }
            }

            if (RECOMBINATION) {
                int crossover_point = random.GetUInt(GENOME_SIZE);
                const ORG_TYPE& parent2 = GetRandomOrg();

                if (random.P(.5) > .5) {
                    for (int i = crossover_point; i < GENOME_SIZE; i++) {
                        org[i] = parent2[i];
                    }
                } else {
                    for (int i = 0; i < crossover_point; i++) {
                        org[i] = parent2[i];
                    }
                }
            }

            return GENOME_SIZE;
        } );

        SetupFitnessFunctions();
        SetupFitnessFile();


    }

    void RunStep() {

        EliteSelect(*this);

        if (SELECTION == "TOURNAMENT") {
            TournamentSelect(*this, TOURNAMENT_SIZE, POP_SIZE-1);
        } else if (SELECTION == "LEXICASE") {
            std::cout << "lex" << std::endl;
            LexicaseSelect(*this, fit_set, POP_SIZE-1);
        } else if (SELECTION == "RESOURCE") {
            ResourceSelect(*this, fit_set, resources, TOURNAMENT_SIZE, POP_SIZE-1, FRAC, MAX_RES_USE, RESOURCE_INFLOW, COST);
        } else if (SELECTION == "ROULETTE") {
            RouletteSelect(*this, POP_SIZE-1);
        } else {
            std::cout << "ERROR: INVALID SELECTION SCHEME: " << SELECTION << std::endl;
            exit(1);
        }
        Update();
        if (isinf(GetFitnessDataNode().GetMax())){
            return;
        }

        DoMutations(1);
        for (auto res : resources) {
            std::cout << res.GetAmount() << " ";
        }
        std::cout << std::endl;
    }

    void Run() {
        for (size_t ud = 0; ud < UPDATES; ud++) {
            RunStep();
            if (isinf(GetFitnessDataNode().GetMax())){
                return;
            }
        }
    }
};
