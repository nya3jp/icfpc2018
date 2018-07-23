After executing sim with `--log` option like
`[icfpc2018]$ bazel-bin/cxx_simulator/sim --target=data/cmodels/FA001_tgt.mdl --trace=FA001.nbt --log=FA001_log.json`,
preprocess the log file `FA001_log.json` by
`[icfpc2018]$ python visualizer/tools/preprocess_log.py FA001_log.json visualizer/data.js`,
and open index.html on a browser.
