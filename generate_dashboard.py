#!/usr/bin/env python3

import json
import os


DATASETS = (
    ('FA', 186),
    ('FD', 186),
    ('FR', 115),
)


def main():
    root_dir = os.path.dirname(__file__)
    data_dir = os.path.join(root_dir, 'data')
    defaults_dir = os.path.join(data_dir, 'defaults')
    traces_dir = os.path.join(data_dir, 'traces')

    print('<!doctype html>')
    print('<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css" integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm" crossorigin="anonymous">')
    print('<style>')
    print('.entry { position: relative; overflow: hidden; margin-bottom: 0.2em; padding: 0.2em; height: 2em; background-color: #eee; }')
    print('.label { position: absolute; left: 0.4em; }')
    print('.bar { float: right; height: 1.6em; background-color: yellow; }')
    print('</style>')
    print('<div class="container-fluid">')
    print('<h1>Team Line Graph</h1>')
    for prefix, count in DATASETS:
        for index in range(1, count + 1):
            name = '%s%03d' % (prefix, index)
            with open(os.path.join(defaults_dir, '%s.json' % name)) as f:
                default_energy = json.load(f)['energy']
            with open(os.path.join(traces_dir, '%s.json' % name)) as f:
                our_energy = json.load(f)['energy']
            percent = '%.2f%%' % (100 - 100 * our_energy / default_energy)
            print('<div class="entry"><div class="label">%s: %s / %s (%s)</div><div class="bar" style="width: %s"></div></div>' % (name, our_energy, default_energy, percent, percent))


if __name__ == '__main__':
    main()
