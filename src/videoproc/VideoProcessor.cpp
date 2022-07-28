#include "VideoProcessor.h"
#include "messages/messages.h"

namespace prm
{
    void VideoProcessor::Init()
    {
        spdlog::info("Initializing video processor module");
        m_messageQueue.Send(PythonWorkerRunString{
                .string = R"(
import pims
import numpy as np
import trackpy as tp

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
sns.set()

from skimage import data, img_as_float
from skimage import exposure
from PIL import Image

import multiprocessing
pythonPath = pythonPath.strip()
multiprocessing.set_executable(pythonPath)

from tifffile import imsave
import os

class Video:
    def __init__(self, path):
        self.path = path
        self.frames = np.array(pims.as_grey(pims.TiffStack(self.path)))
        #self.frames, _ = subtract_background_rolling_ball(self.frames, 30, light_background=False,
        #                 use_paraboloid=False, do_presmooth=True)

        self.frames_rescale = self.frames
        self.minmass = 1e5
        self.f = pd.DataFrame()
        self.t = pd.DataFrame()
        self.d = pd.DataFrame()
        self.raw_t = pd.DataFrame()
        self.up_perc = 99.99

    def __str__(self):
        return "Video"

    def locate_1_frame(self, num, ecc, mass,size, diam = 15):
        f = tp.locate(self.frames_rescale[num], diam, minmass = self.minmass)
        self.f = f[(f['ecc'] < ecc)&(f['mass'] > mass)&(f['size'] < size)]

        fig, ax = plt.subplots()
        fig.set_figheight(10)
        fig.set_figwidth(20)
        tp.annotate(self.f, self.frames_rescale[num], ax = ax, imshow_style = {'cmap':'viridis'})
        plt.savefig('plot.png', bbox_inches='tight')

    def locate_all(self, ecc, mass, size, diam = 15):
        f = tp.batch(self.frames_rescale, diam, processes="auto", minmass=self.minmass)
        self.f = f[(f['ecc'] < ecc)&(f['mass'] > mass)&(f['size'] < size)]

    def link(self,  ecc, mass, size,search_range = 10, memory = 3):
        self.raw_t = tp.link(self.f, search_range, memory=memory)
        self.t = self.raw_t[(self.raw_t['ecc'] < ecc)&(self.raw_t['mass'] > mass)&(self.raw_t['size'] < size)]

    def filter_traj(self, min_len = 20):
        self.t = tp.filter_stubs(self.raw_t, min_len)
        print('Before:', self.raw_t['particle'].nunique())
        print('After:', self.t['particle'].nunique())
)",
                .strVariables{{"pythonPath", m_pythonExePath}}});
    }

    void VideoProcessor::LoadVideo(std::string_view path)
    {
        vidPath = std::filesystem::path{path};
        if (!std::filesystem::exists(vidPath))
        {
            spdlog::error("No such file, please check the path");
            return;
        }

        spdlog::info("Instantiating Video Class");

        m_messageQueue.Send(PythonWorkerRunString{
                .string = R"(
vid = Video(path)
plot = False
)",
                .strVariables{{"path", std::string{path}}}});
    }

    void VideoProcessor::LocateOneFrame(int frameNum, int minm, double ecc,
                                        int size, int diameter)
    {
        spdlog::info("Locating features on one frame");
        m_messageQueue.Send(
                PythonWorkerRunString{.string = R"(
vid.minmass = minm
mass = minm
#Эта функция для подбора параметров на одном кадре. Изменяем параметры (в основном, mass),
#пока картинка не станет хорошей, и только тогда запускаем locate_all
vid.locate_1_frame(frameNum, ecc, mass, size, diam=diameter)
plot = True
)",
                                      .intVariables{{"frameNum", frameNum},
                                                    {"minm", minm},
                                                    {"size", size},
                                                    {"diameter", diameter}},
                                      .floatVariables{{"ecc", ecc}}});
    }

    void VideoProcessor::LocateAllFrames()
    {
        spdlog::info("Locating features for all frames");
        m_messageQueue.Send(PythonWorkerRunString{.string = R"(
import time
starttime = time.time()
vid.locate_all(ecc, mass, size, diam=diameter)
print(time.time() - starttime)
plot = False
)"});
    }

    void VideoProcessor::LinkAndFilter(int searchRange, int memory,
                                       int minTrajectoryLen, int driftSmoothing)
    {
        spdlog::info(
                "Linking features, filtering trajectory and subtracting drift");
        m_messageQueue.Send(PythonWorkerRunString{
                .string = R"(
vid.raw_t = tp.link_df(vid.f, search_range, memory = mem)

vid.filter_traj(min_len = min_traj_len)

d = tp.compute_drift(vid.t, smoothing = drift_smoothing)
vid.t = tp.subtract_drift(vid.t, d)
plot = False
)",
                .intVariables{{"search_range", searchRange},
                              {"mem", memory},
                              {"min_traj_len", minTrajectoryLen},
                              {"drift_smoothing", driftSmoothing}}});
    }

    void VideoProcessor::GroupAndPlotTrajectory(int minDiagSize,
                                                int maxDiagSize)
    {
        spdlog::info("Grouping by particle, filtering by diagonal \ntrajectory "
                     "size, plotting trajectory");
        m_messageQueue.Send(PythonWorkerRunString{
                .string{R"(
vid.t = vid.t.groupby('particle').filter(lambda x: tp.diagonal_size(x) > min_diag_size and tp.diagonal_size(x) < max_diag_size)

fig, ax = plt.subplots()
fig.set_figheight(10)
fig.set_figwidth(20)
tp.plot_traj(vid.t, ax=ax)
plt.savefig('plot.png', bbox_inches='tight')
plot = True
)"},
                .intVariables{{"min_diag_size", minDiagSize},
                              {"max_diag_size", maxDiagSize}}});
    }

    void VideoProcessor::PlotSizeHist(double fps, double scale, int bins)
    {
        spdlog::info("Plotting the size distribution");
        m_messageQueue.Send(PythonWorkerRunString{
                .string{R"(
im = tp.imsd(vid.t, scale, fps)
sizes = []
for i in im.columns:
    res = tp.utils.fit_powerlaw(im[i].dropna(), plot = False)
    A = float(res['A'])
    eta = 0.89e-3
    k = 1.380649e-23
    T = 273 + 25
    D = 1e-12*A/4
    R = k*T/(6*np.pi*D*eta)
    diam = 2*R*1e9
    sizes.append(diam)
sizes = np.array(sizes)
sizes = sizes[~np.isnan(sizes)]
# the histogram of the data
fig, ax = plt.subplots()
fig.set_figheight(10)
fig.set_figwidth(20)
n, bins, patches = ax.hist(sizes, num_bins, facecolor='blue', alpha=0.5)

mean = np.mean(sizes)
median = np.median(sizes)
std = np.std(sizes)

plt.xlim([0, 1000])
plt.text(0.85, 0.85, 'average: {} \n median: {} \n std: {}'.format(mean, median, std), transform=plt.gca().transAxes)
plt.savefig('plot.png', bbox_inches='tight')
plt.savefig(dir_path + "\\" + 'hist.png', bbox_inches='tight')

hist = pd.DataFrame({'n':np.append(n, 0), 'bins':bins})
hist.to_csv(dir_path + "\\" + file_stem + "_hist.csv", index = False)
pd.Series(sizes).to_csv(dir_path + "\\" + file_stem + '_raw_data.csv', index = False)
plot = True
)"},
                .strVariables{{"dir_path", vidPath.parent_path().string()},
                              {"file_stem", vidPath.stem().string()}},
                .intVariables{{"num_bins", bins}},
                .floatVariables = {{"scale", scale}, {"fps", fps}}});
    }

    void VideoProcessor::RunPythonQuery(std::string_view query)
    {
        spdlog::info("Running a python string");
        m_messageQueue.Send(
                PythonWorkerRunString{.string = std::string{query}});
    }
}// namespace prm