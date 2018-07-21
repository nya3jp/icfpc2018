c = document.getElementById('glcanvas');
s = document.getElementById('stdout');

function setAngle() {
  c.dispatchEvent(new KeyboardEvent('keypress', {key: 'r'}));
  for (let i = 1; i < 12; i++) {
    c.dispatchEvent(new KeyboardEvent('keypress', {key: 's'}));
    c.dispatchEvent(new KeyboardEvent('keypress', {key: 'd'}));
  }
}

function download(name) {
  vis.render();
  a = document.createElement('a');
  a.href = c.toDataURL('image/png');
  a.download = name;
  a.click();
}

c.addEventListener('dragover', (e) => { e.preventDefault(); });

c.addEventListener('drop', (e) => {
  e.preventDefault();
  document.getElementById('modelFileIn').files = e.dataTransfer.files;
  const pngName = e.dataTransfer.files[0].name + '.png';
  const poll = () => {
    if (s.innerHTML.indexOf('Success::') >= 0) {
      setAngle();
      setTimeout(() => {
        download(pngName);
      }, 100);
    } else {
      setTimeout(poll, 100);
    }
  };
  setTimeout(poll, 200);
});
