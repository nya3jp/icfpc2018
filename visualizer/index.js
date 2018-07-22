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

const box_size = 8 / resolution;
const grid_size = 10 / resolution;
const geometry_box = new THREE.BoxGeometry(box_size, box_size, box_size);

function generate_box(xi, yi, zi) {
    const material_box = new THREE.MeshStandardMaterial({color: 0xffff00});
    const box = new THREE.Mesh(geometry_box, material_box);
    x = xi * grid_size;
    y = yi * grid_size;
    z = zi * grid_size;
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

function generate_scene(width, height, renderer) {
    const scene = new THREE.Scene();

    // Draw grid lines.
    var material_line = new THREE.LineBasicMaterial({
        color: 0xffffff,
        transparent: true,
        opacity: 0.2
    });
    start = - grid_size / 2;
    end = resolution * grid_size - grid_size / 2;
    function add_line(x0, y0, z0, x1, y1, z1) {
        var geometry_line = new THREE.Geometry();
        geometry_line.vertices.push(
            new THREE.Vector3(x0, y0, z0),
            new THREE.Vector3(x1, y1, z1)
        );
        var line = new THREE.Line(geometry_line, material_line);
        scene.add(line);
    }
    if (true) {  // Line drawing is heavy in large resolution.
        stride = resolution
    } else {
        stride = 1
    }
    for (var i = 0; i <= resolution; i += stride) {
        for (var j = 0; j <= resolution; j += stride) {
            ci = (i - 0.5) * grid_size;
            cj = (j - 0.5) * grid_size;
            add_line(start, ci, cj, end, ci, cj);
            add_line(ci, start, cj, ci, end, cj);
            add_line(ci, cj, start, ci, cj, end);
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

    var scene = generate_scene(width, height, renderer);
    var boxes = Array(resolution * resolution * resolution);
    console.log(scene)

    function changeFrame(i) {
        for (var j = 0; j < simlog[i].length; j++) {
            visual = simlog[i][j][0];
            color = simlog[i][j][1];
            if (visual == 'visible' & boxes[j] == undefined) {
                coord_index = g2c(j);
                box = generate_box(coord_index[0],
                                   coord_index[1],
                                   coord_index[2]);
                boxes[j] = box;
                scene.add(box);
            }
            if (visual == 'visible') {
                boxes[j].visible = true;
                boxes[j].material.color.setHex(color);
            } else if (visual == 'invisible') {
                boxes[j].visible = false;
            } else if (visual == 'dispose') {
                boxes[j].material.dispose();
                scene.remove(boxes[j]);
                boxes[j] = undefined;
            }
        }
    }

    var slider = document.getElementById("slider");
    slider.setAttribute("max", simlog.length - 1);
    slider.addEventListener("input", changeFrameFromSlider);

    function changeFrameFromSlider(e){
        var i = e.target.value;
        changeFrame(i);
    }

    changeFrame(0);
    tick();

    function tick() {
        renderer.render(scene, camera);
        requestAnimationFrame(tick);
    }
}
