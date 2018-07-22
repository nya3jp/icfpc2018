const fs = require('fs');
const process = require('process');

const PRESENT_ELEMENT_IDS = [
  'tgtModelFileIn',
  'traceFileIn',
  'stepsPerFrame',
  'execTrace',
  'stdout',
  'full',
  'srcModelFileIn',
  'srcModelEmpty',
  'tgtModelEmpty',
];
const ABSENT_ELEMENT_IDS = [
];

class MockElement {
  constructor(id) {
    this.id_ = id;
    this.htmlObservers_ = [];
  }

  addHtmlObserver(callback) {
    this.htmlObservers_.push(callback);
  }

  addEventListener() {
    this.notImplemented_('addEventListener');
  }

  setAttribute(key, value) {
    //this.log_(`setAttribute("${key}", "${value}")`);
  }
  removeAttribute(key, value) {
    //this.log_(`removeAttribute("${key}", "${value}")`);
  }

  get innerHTML() {
    //this.log_('get innerHTML');
  }
  set innerHTML(html) {
    //this.log_(`set innerHTML: ${html}`);
    for (const callback of this.htmlObservers_) {
      callback(html);
    }
  }

  get innerText() {
    this.notImplemented_('get innerText');
  }
  set innerText(text) {
    this.notImplemented_(`set innerText: ${text}`);
  }

  /*
  get value() {
    this.notImplemented_('get value');
  }
  set value(val) {
    this.notImplemented_(`set value: ${val}`);
  }
  */

  /*
  get onclick() {
    this.notImplemented_('get onclick');
  }
  set onclick(html) {
    this.notImplemented_('set onclick');
  }
  */

  log_(name) {
    console.log(`MockElement(${this.id_}): ${name}`);
  }

  notImplemented_(name) {
    throw new Error(`MockElement(${this.id_}): not implemented: ${name}`);
  }
}

const mockElementProxy = {
  get: function(target, name) {
    if (name.lastIndexOf('_') !== name.length - 1) {
      console.log(`PROXY: get MockElement(${target.id_}).${name}`);
    }
    return target[name];
  },
  /*
  set: function(target, name, value) {
    if (name.lastIndexOf('_') !== name.length - 1) {
      console.log(`PROXY: set MockElement(${target.id_}).${name} = ${value}`);
    }
    target[name] = value;
  },
  */
};

class MockDocument {
  constructor() {
    this.elements_ = MockDocument.createElements_();
  }

  getElementById(id) {
    if (!(id in this.elements_)) {
      throw new Error(`MockDocument.getElementById: no such element: ${id}`);
    }
    //console.log(`MockDocument.getElementById: ${id}`);
    return this.elements_[id];
  }

  static createElements_() {
    const elements = {};
    for (const id of PRESENT_ELEMENT_IDS) {
      //elements[id] = new Proxy(new MockElement(id), mockElementProxy);
      elements[id] = new MockElement(id);
    }
    for (const id of ABSENT_ELEMENT_IDS) {
      elements[id] = null;
    }
    return elements;
  }
}

const mockDocumentProxy = {
  get: function(target, name) {
    if (name.lastIndexOf('_') !== name.length - 1) {
      console.log(`PROXY: get MockDocument(${target.id_}).${name}`);
    }
    return target[name];
  },
  /*
  set: function(target, name, value) {
    if (name.lastIndexOf('_') !== name.length - 1) {
      console.log(`PROXY: set MockDocument(${target.id_}).${name} = ${value}`);
    }
    target[name] = value;
  },
  */
};

// Global variables.
vis = null;
//document = new Proxy(new MockDocument(), mockDocumentProxy);
document = new MockDocument();
bdataLength = function(data) { return data.length; };
bdataSub = function(data, i) { return data[i]; };
requestAnimationFrame = function(callback) { setTimeout(callback, 0); };

// Load the simulator source.
require('../third_party/official/exec-trace.js');

function onUpdate(html) {
  console.error(html);
  if (html.length === 0 ||
      html.indexOf('Loading') === 0 ||
      html.indexOf('Executing') === 0) {
    return;
  }
  if (html.indexOf('Success::') === 0) {
    const time = /Time:\s*(\d+)/.exec(html)[1];
    const energy = /Energy:\s*(\d+)/.exec(html)[1];
    console.log(`${time}\t${energy}`);
  } else {
    process.exit(1);
  }
}

function main() {
  if (process.argv.length !== 5) {
    console.error('Usage: simulator <srcmodel> <tgtmodel> <trace>');
    return;
  }

  const srcModelPath = process.argv[2];
  const tgtModelPath = process.argv[3];
  const tracePath = process.argv[4];

  if (srcModelPath !== 'null') {
    document.getElementById('srcModelEmpty').checked = false;
    srcModelBData = new Uint8Array(fs.readFileSync(srcModelPath));
  } else {
    document.getElementById('srcModelEmpty').checked = true;
    srcModelBData = null;
  }

  if (tgtModelPath !== 'null') {
    document.getElementById('tgtModelEmpty').checked = false;
    tgtModelBData = new Uint8Array(fs.readFileSync(tgtModelPath));
  } else {
    document.getElementById('tgtModelEmpty').checked = true;
    tgtModelBData = null;
  }

  traceBData = new Uint8Array(fs.readFileSync(tracePath));

  document.getElementById('full').value = 'true';
  document.getElementById('stepsPerFrame').value = '1000000000';
  document.getElementById('stdout').addHtmlObserver(onUpdate);
  document.getElementById('execTrace').onclick();
}

main();
