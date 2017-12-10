//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "web/Selector.h"
#include "web/color_map.h"
#include "../box-world.h"
#include "web/d3/visualizations.h"

namespace UI = emp::web;

struct BoxInterface {
    BoxWorld world;

    UI::Document doc;
    UI::Div div_pop;
    UI::Div div_stats;
    UI::Div div_controls;
    UI::Div div_vis;

    UI::Canvas pop_canvas;
    UI::Canvas org_canvas;
    UI::Animate anim;

    UI::HistogramChart<double> d1;
    UI::HistogramChart<double> d2;
    UI::HistogramChart<double> d3;
    UI::HistogramChart<double> d4;
    UI::HistogramChart<double> d5;
    UI::HistogramChart<double> d6;
    UI::HistogramChart<double> d7;
    UI::HistogramChart<double> d8;
    UI::HistogramChart<double> d9;
    UI::HistogramChart<double> d10;

    UI::Selector selection_scheme;
    UI::TextArea n_good;
    UI::TextArea n_bad;
    UI::TextArea resource_inflow;
    UI::TextArea mutation_size;
    UI::TextArea pop_size;
    UI::TextArea recombination;
    UI::TextArea distance_cutoff;
    UI::TextArea cost;
    UI::TextArea frac;
    UI::TextArea max_res_use;
    UI::TextArea tournament_size;

    emp::vector<emp::vector<double> > data;

    void SetupData() {
        data.resize(world.GENOME_SIZE);
        for (auto & vec : data) {
            vec.resize(world.GetSize());
        }
    }

    BoxInterface()
        : doc("emp_base")
        , div_pop("div_pop"), div_stats("div_stats"), div_controls("div_controls"), div_vis("div_vis")
        , pop_canvas(400, 400, "pop_canvas"), org_canvas(800, 800, "org_canvas")
        , anim( [this](){ DoFrame(); }, org_canvas ), d1("d1"), d2("d2"), d3("d3")
        , d4("d4"), d5("d5"), d6("d6"), d7("d7"), d8("d8"), d9("d9"), d10("d10")
        , selection_scheme("selection_scheme"), n_good("n_good"), n_bad("n_bad")
        , resource_inflow("resource_inflow"), mutation_size("mutation_size")
        , pop_size("pop_size"), recombination("recombination"), distance_cutoff("distance_cutoff")
        , cost("cost"), frac("frac"), max_res_use("max_res_use"), tournament_size("tournament_size")
    {
        world.Setup();

        SetupData();

        selection_scheme.SetOption("Tournament", [this](){world.config.SELECTION("TOURNAMENT"); world.SELECTION = "TOURNAMENT";});
        selection_scheme.SetOption("Lexicase", [this](){world.config.SELECTION("LEXICASE"); world.SELECTION = "LEXICASE";});
        selection_scheme.SetOption("Resource", [this](){world.config.SELECTION("RESOURCE"); world.SELECTION = "RESOURCE";});
        selection_scheme.SetOption("Roulette", [this](){world.config.SELECTION("ROULETTE"); world.SELECTION = "ROULETTE";});

        n_good.SetCallback([this](const std::string & curr){world.config.N_GOOD(emp::from_string<int>(curr)); world.N_GOOD = world.config.N_GOOD(); world.SetupFitnessFunctions();});
        n_bad.SetCallback([this](const std::string & curr){world.config.N_BAD(emp::from_string<int>(curr)); world.N_BAD = world.config.N_BAD(); world.SetupFitnessFunctions();});
        resource_inflow.SetCallback([this](const std::string & curr){world.config.RESOURCE_INFLOW(emp::from_string<double>(curr)); world.RESOURCE_INFLOW = world.config.RESOURCE_INFLOW();});
        mutation_size.SetCallback([this](const std::string & curr){world.config.MUTATION_SIZE(emp::from_string<double>(curr)); world.InitConfigs();});
        pop_size.SetCallback([this](const std::string & curr){world.config.POP_SIZE(emp::from_string<int>(curr)); world.InitConfigs(); world.InitPop(); SetupData();});
        recombination.SetCallback([this](const std::string & curr){world.config.RECOMBINATION(emp::from_string<int>(curr)); world.InitConfigs();});
        distance_cutoff.SetCallback([this](const std::string & curr){world.config.DISTANCE_CUTOFF(emp::from_string<double>(curr)); world.InitConfigs();});
        cost.SetCallback([this](const std::string & curr){world.config.COST(emp::from_string<double>(curr)); world.InitConfigs();});
        frac.SetCallback([this](const std::string & curr){world.config.FRAC(emp::from_string<double>(curr)); world.InitConfigs();});
        max_res_use.SetCallback([this](const std::string & curr){world.config.MAX_RES_USE(emp::from_string<double>(curr)); world.InitConfigs();});
        tournament_size.SetCallback([this](const std::string & curr){world.config.TOURNAMENT_SIZE(emp::from_string<int>(curr)); world.InitConfigs();});

        div_pop.SetSize(100,400).SetScrollAuto();

        // Attach the GUI components to the web doc.
        div_controls << UI::Button( [this](){ world.RunStep(); DrawAll(); }, "Step", "but_step" );
        div_controls << anim.GetToggleButton("but_toggle");
        div_controls << UI::Button( [this](){ world.Reset(); world.Setup(); DrawAll(); }, "Reset", "but_reset");
        div_controls << selection_scheme;
        div_controls << " n_good: " << n_good;
        div_controls << " n_bad: " << n_bad;
        div_controls << " resource_inflow: " << resource_inflow;
        div_controls << " mutation_size: " << mutation_size;
        div_controls << " pop_size: " << pop_size;
        div_controls << " recombination: " << recombination;
        div_controls << " distance_cutoff: " << distance_cutoff;
        div_controls << " cost: " << cost;
        div_controls << " frac: " << frac;
        div_controls << " max_res_use: " << max_res_use;
        div_controls << " tournament_size: " << tournament_size;

        div_pop << org_canvas;

        auto & fit_node = world.GetFitnessDataNode();
        div_stats << "<b>Stats:</b>";
        div_stats << "<br>Update: " << UI::Live( [this](){ return world.GetUpdate(); } );
        div_stats << "<br>Min Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMin(); } );
        div_stats << "<br>Mean Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMean(); } );
        div_stats << "<br>Max Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMax(); } );

        div_vis << d1;
        div_vis << d2;
        div_vis << d3;
        div_vis << d4;
        div_vis << d5;
        div_vis << d6;
        div_vis << d7;
        div_vis << d8;
        div_vis << d9;
        div_vis << d10;

        doc << "<h1>Box problem</h1>";
        doc << div_pop;
        doc << div_stats;
        doc << div_controls;
        doc << div_vis;

        UI::OnDocumentReady( [this](){ LayoutDivs(); DrawAll(); } );
    }

    void LayoutDivs() {
      const double spacing = 10;
      const double x1 = div_pop.GetXPos();
      const double x2 = x1 + div_pop.GetOuterWidth() + spacing;
      const double y1 = div_pop.GetYPos();
      const double y2 = y1 + div_controls.GetOuterHeight() + spacing;
      const double y3a = y1 + div_pop.GetOuterHeight();
      const double y3b = y2 + div_stats.GetOuterHeight();
      const double y3 = emp::Max(y3a, y3b) + spacing;
      div_controls.SetPosition(x2, y1);
      div_stats.SetPosition(x2, y2);
      div_vis.SetPosition(x1,y3);
    }

    void DrawAll() {
    //   DrawOrgs();
      div_stats.Redraw();

      if (anim.GetFrameCount() % 1 == 0 ) {
          org_canvas.SetSize(6*world.PROBLEM_DIMENSIONS, 6 * world.GetSize());
          org_canvas.Clear("black");

          for (size_t id = 0; id < world.GetSize(); id++) {
              auto & org = world[id];
              for (size_t pos = 0; pos < org.size(); pos++) {
                  data[pos][id] = org[pos];
                  org_canvas.Rect(pos*6, id*6, 6, 6, emp::ColorHSL(org[pos]*360, 100, 50));
              }
          }

          d1.DrawData(data[0]);
          d2.DrawData(data[1]);
          d3.DrawData(data[2]);
          d4.DrawData(data[3]);
          d5.DrawData(data[4]);
          d6.DrawData(data[5]);
          d7.DrawData(data[6]);
          d8.DrawData(data[7]);
          d9.DrawData(data[8]);
          d10.DrawData(data[9]);
      }

      LayoutDivs();
    }

    void DoFrame() {
        world.RunStep();
        DrawAll();
    }

};

BoxInterface interface;

int main()
{
}
