#!/usr/bin/env python3

import datetime
import json
import os
import shutil
import subprocess
import sys
import tempfile
import traceback


SCRIPT_URL = 'https://script.google.com/macros/s/AKfycbzQ7Etsj7NXCN5thGthCvApancl5vni5SFsb1UoKgZQwTzXlrH7/exec'
WEBHOOK_URL = 'https://hooks.slack.com/services/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'


def _post_to_slack(msg):
    subprocess.check_call([
        'curl', '-X', 'POST',
        '--data-urlencode', 'payload=%s' % json.dumps({'text': msg}),
        WEBHOOK_URL,
    ])


def _real_main():
    www_dir, base_url = sys.argv[1:]
    traces_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'data', 'traces'))
    timestamp = (datetime.datetime.utcnow() + datetime.timedelta(hours=9)).strftime('%Y%m%d-%H%M%S')

    zip_path = os.path.join(www_dir, '0123-%s.zip' % timestamp)

    with open(os.path.join(os.environ['HOME'], '.icfpc2018.key'), 'r') as f:
        private_id = f.read().strip()

    with tempfile.TemporaryDirectory() as tmp_dir:
        os.chdir(tmp_dir)
        for gz_name in os.listdir(traces_dir):
            if not gz_name.endswith('.nbt.gz'):
                continue
            name = gz_name.split('.', 1)[0] + '.nbt'
            with open(os.path.join(traces_dir, gz_name), 'rb') as fin:
                with open(os.path.join(tmp_dir, name), 'wb') as fout:
                    subprocess.check_call(['gzip', '-d'], stdin=fin, stdout=fout)
        with tempfile.NamedTemporaryFile(suffix='.zip') as tmp_file:
            os.unlink(tmp_file.name)
            subprocess.check_call(['zip', '-q', '-r', '-e', '-P', private_id, tmp_file.name, '.'])
            shutil.copy(tmp_file.name, zip_path)

    subprocess.check_call(['unzip', '-l', '-P', private_id, zip_path])
    zip_hash = subprocess.check_output(['shasum', '-a', '256', zip_path]).decode('utf-8').split()[0]
    print('SHA256:', zip_hash)
    zip_url = base_url.rstrip('/') + '/' + os.path.basename(zip_path)
    print('URL:', zip_url)

    output = subprocess.check_output([
        'curl',
        '-L',
        '--data-urlencode', 'action=submit',
        '--data-urlencode', 'privateID=%s' % private_id,
        '--data-urlencode', 'submissionURL=%s' % zip_url,
        '--data-urlencode', 'submissionSHA=%s' % zip_hash,
        SCRIPT_URL,
    ])

    _post_to_slack(output.decode('utf-8'))


def main():
    try:
        _real_main()
    except Exception:
        _post_to_slack('<!channel> ' + traceback.format_exc())
        raise


if __name__ == '__main__':
    main()
