#ifndef PLOT_HPP_INCLUDED
#define PLOT_HPP_INCLUDED



#endif // PLOT_HPP_INCLUDED

#include "gnuplot_i.hpp"
#include <iostream>

void plot_data(
               vector<double> frame_energy,
               vector<double> frame_freq_peak,
               vector<double> frame_spectralflatness,
               vector<double> frame_zerocrossing_rate,
               vector<int>    VADresult
               ){
    //VAD test
    Gnuplot VADplot;
   VADplot.cmd(" set multiplot layout 5, 1 title \"Multiplot VAD\" font \",14\" ");
    VADplot.cmd(" set tmargin 2 ");
        //plot energy
    VADplot.cmd(" set title \"Frame Energy\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_energy,"Frame Energy");

       //plot max freq
    VADplot.cmd(" set title \"Frame Max Frequency\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_freq_peak,"Frame Max Freq");
        //plot SF
    VADplot.cmd(" set title \"Frame Spectral Flatness\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_spectralflatness,"Frame SFM");
        //plot zero crossing
    VADplot.cmd(" set title \"Frame Zero-Crossing Rate\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_zerocrossing_rate,"Frame ZCR");
        //plotVAD Result
    VADplot.cmd(" set title \"Frame VAD\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("impulses").plot_x(VADresult,"VAD result");
    VADplot.cmd(" unset multiplot ");
    VADplot.showonscreen();
    getch();
}

void plot_boolean(
                    vector<int> energy_passed,
                    vector<int> freq_passed,
                    vector<int> sfm_passed,
                    vector<int> zcr_passed,
                    vector<int>    VADresult
                  ){
// plot boolean of feature
     Gnuplot VADplot_bool;
    VADplot_bool.cmd(" set multiplot layout 5, 1 title \"Multiplot VAD bool\" font \",14\" ");
        //plot energy
    VADplot_bool.cmd(" set title \"Frame Energy\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(energy_passed,"Frame Energy");
        //plot max freq
    VADplot_bool.cmd(" set title \"Frame Max Frequency\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(freq_passed,"Frame Max Freq");
        //plot SF
    VADplot_bool.cmd(" set title \"Frame Spectral Flatness\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(sfm_passed,"Frame SFM");
        //plot zero crossing
    VADplot_bool.cmd(" set title \"Frame Zero-Crossing Rate\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(zcr_passed,"Frame ZCR");
        //plotVAD Result
    VADplot_bool.cmd(" set title \"Frame VAD\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(VADresult,"VAD result");
    VADplot_bool.cmd(" unset multiplot ");

    VADplot_bool.showonscreen();
    getch();
}


