// Documents
// - https://ics.media/tutorial-three/index.html
//   - Nice introduction for Three.js
// - https://ics.media/entry/14771
//   - Similar to above
// - https://jsfiddle.net/prisoner849/vo1urg68/
//   - Three.js example with slider
// - https://codepen.io/dxinteractive/pen/reNpOR
//   - Three.js with 2D HTML text labels

window.addEventListener('DOMContentLoaded', init);

const boxSize = 8 / resolution;
const gridSize = 10 / resolution;
const geometryBox = new THREE.BoxGeometry(boxSize, boxSize, boxSize);

function generateBox(xi, yi, zi) {
    const materialBox = new THREE.MeshStandardMaterial({color: 0xffff00});
    const box = new THREE.Mesh(geometryBox, materialBox);
    x = xi * gridSize;
    y = yi * gridSize;
    z = zi * gridSize;
    box.castShadow = true;
    box.receiveShadow = true;
    box.position.set(x, y, z);
    return box;
}

function g2c(i) {
    zi = i % resolution;
    yi = (i - zi) / resolution % resolution;
    xi = (i - zi - yi * resolution) / (resolution * resolution);
    return [xi, yi, zi];
}

function c2g(xi, yi, zi) {
    return xi * resolution * resolution + yi * resolution + zi;
}

function generateScene(width, height, renderer) {
    const scene = new THREE.Scene();

    // Draw grid lines.
    var materialLine = new THREE.LineBasicMaterial({
        color: 0xffffff,
        transparent: true,
        opacity: 0.2
    });
    start = - gridSize / 2;
    end = resolution * gridSize - gridSize / 2;
    function addLine(x0, y0, z0, x1, y1, z1) {
        var geometryLine = new THREE.Geometry();
        geometryLine.vertices.push(
            new THREE.Vector3(x0, y0, z0),
            new THREE.Vector3(x1, y1, z1)
        );
        var line = new THREE.Line(geometryLine, materialLine);
        scene.add(line);
    }
    if (true) {  // Line drawing is heavy in large resolution.
        stride = resolution
    } else {
        stride = 1
    }
    for (var i = 0; i <= resolution; i += stride) {
        for (var j = 0; j <= resolution; j += stride) {
            ci = (i - 0.5) * gridSize;
            cj = (j - 0.5) * gridSize;
            addLine(start, ci, cj, end, ci, cj);
            addLine(ci, start, cj, ci, end, cj);
            addLine(ci, cj, start, ci, cj, end);
        }
    }

    const light1 = new THREE.PointLight();
    const light2 = new THREE.PointLight();
    light1.intensity = 2;
    light2.intensity = 2;
    //light1.castShadow = true;
    //light2.castShadow = true;
    light1.position.set(-15, -15, -15);
    light2.position.set(15, 15, 15);
    scene.add(light1);
    scene.add(light2);

    return scene;
}

function init() {
    const width = 960;
    const height = 540;
    const renderer = new THREE.WebGLRenderer({
        canvas: document.querySelector('#myCanvas')
    });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(width, height);
    renderer.shadowMap.enabled = true;

    const camera = new THREE.OrthographicCamera(width / - 40, width / 40, height / 40, height / - 40, 0.1, 100);
    camera.position.set(15, 15, 15);
    document.body.appendChild(renderer.domElement);
    const controls = new THREE.OrbitControls(camera, renderer.domElement);

    var scene = generateScene(width, height, renderer);
    var boxesMatrix = Array(resolution * resolution * resolution);
    var boxesBots = [];
    var tick = -1;

    function drawMatrixFromDiffs(diffs) {
        for (var j = 0; j < diffs.length; j++) {
            index = diffs[j][0];
            visual = diffs[j][1];
            color = diffs[j][2];
            if (visual & boxesMatrix[index] == undefined) {
                coordIndex = g2c(index);
                box = generateBox(coordIndex[0],
                                  coordIndex[1],
                                  coordIndex[2]);
                boxesMatrix[index] = box;
                scene.add(box);
            }
            if (visual) {
                boxesMatrix[index].material.color.setHex(color);
            } else {
                scene.remove(boxesMatrix[index]);
                boxesMatrix[index] = undefined;
            }
        }
    }

    function disposeBoxesBots() {
        for (var i = 0; i < boxesBots.length; i++) {
            scene.remove(boxesBots[i]);
            boxesBots[i] = undefined;
        }
    }

    function drawBots(bots) {
        for (var i = 0; i < bots.length; i++) {
            coordIndex = bots[i][0];
            color = bots[i][1];
            box = generateBox(coordIndex[0],
                              coordIndex[1],
                              coordIndex[2]);
            boxesBots.push(box);
            box.material.color.setHex(color);
            scene.add(box);
        }
    }

    function changeTick(nextTick) {
        console.log("changeTick", tick, "to", nextTick);
        if (tick < nextTick) {
            for (var t = tick + 1; t < nextTick; t++) {
                drawMatrixFromDiffs(diffsForward[t]);
            }
        } else if (tick > nextTick) {
            for (var t = tick - 1; t >= nextTick; t--) {
                drawMatrixFromDiffs(diffsBackward[t]);
            }
        }
        disposeBoxesBots();
        drawBots(botStates[nextTick]);
        tick = nextTick;
    }

    var slider = document.getElementById("slider");
    slider.setAttribute("max", diffsForward.length - 1);
    slider.addEventListener("input", changeTickFromSlider);

    function changeTickFromSlider(e) {
        var i = Number(e.target.value);
        changeTick(i);
    }

    changeTick(0);
    show();

    function show() {
        renderer.render(scene, camera);
        requestAnimationFrame(show);
    }
}
