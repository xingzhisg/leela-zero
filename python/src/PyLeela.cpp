#include <algorithm>

#include <boost/python.hpp>

#include "GTP.h"
#include "Utils.h"
#include "Random.h"
#include "Zobrist.h"

#include "advisor.h"
#include "nogil.h"

namespace py = boost::python;


size_t thread_count_cpu() {
    // If we are CPU-based, there is no point using more than the number of CPUs.
    return std::min(SMP::get_num_cpus(), size_t{MAX_CPUS});
}

#ifdef USE_OPENCL
size_t thread_count_gpu() {
    auto cfg_max_threads = size_t{MAX_CPUS};
    auto gpu_count = std::max(cfg_gpus.size(), size_t(1));
    cfg_batch_size = 5;

    return std::min(cfg_max_threads, cfg_batch_size * gpu_count * 2);
}
#endif

// Setup global objects after command line has been parsed
void init_global_objects() {

#ifdef USE_OPENCL
    cfg_num_threads = thread_count_gpu();
    Utils::myprintf("Using OpenCL batch size of %d\n", cfg_batch_size);
#else
    cfg_num_threads = thread_count_cpu();
    Utils::myprintf("Using %d CPU threads\n", cfg_num_threads);
#endif

    thread_pool.initialize(cfg_num_threads);

    // Use deterministic random numbers for hashing
    auto rng = std::make_unique<Random>(5489);
    Zobrist::init_zobrist(*rng);

    // Initialize the main thread RNG.
    // Doing this here avoids mixing in the thread_id, which
    // improves reproducibility across platforms.
    Random::get_Rng().seedrandom(cfg_rng_seed);

    Utils::create_z_table();
}


void initialize() {
    GTP::setup_default_parameters();

    init_global_objects();
}


class LeelaZeroAdvisor : public leelapy::Advisor {
public:
    LeelaZeroAdvisor(PyObject* publish_output, py::str weights_file)
    : Advisor([this](const char* res) {this->publish(res);}
              , py::extract<std::string>(weights_file)
              )
    , m_publish_output(publish_output) 
    {}

    bool analyze(py::str sgfStr, int interval_msec) {
        Advisor::clear_cache();
        Advisor::loadsgf(py::extract<std::string>(sgfStr));
        return Advisor::lz_analyze(interval_msec);
    }

    void publish(const char* res) {
        with_gil gil;

        PyObject* args = PyTuple_Pack(1, Py_BuildValue("s", res));
        PyObject* myResult = PyObject_CallObject(m_publish_output, args);
    }

private:
    PyObject* m_publish_output;
};


BOOST_PYTHON_MODULE(leelapy)
{
    Py_Initialize();
    PyEval_InitThreads();

    initialize();

    py::class_<LeelaZeroAdvisor>("LeelaZeroAdvisor", py::init<PyObject*, py::str>())
        .def("analyze", &LeelaZeroAdvisor::analyze)
        .def("stop_analysis", &LeelaZeroAdvisor::stop_analysis)
    ;
}
