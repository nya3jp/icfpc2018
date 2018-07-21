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

function init() {
    const width = 960;
    const height = 540;

    const renderer = new THREE.WebGLRenderer({
        canvas: document.querySelector('#myCanvas')
    });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(width, height);
    renderer.shadowMap.enabled = true;

    const scene = new THREE.Scene();

    const camera = new THREE.OrthographicCamera(width / - 2, width / 2, height / 2, height / - 2, 1, 100000);
    camera.position.set(5000, 5000, 5000);

    document.body.appendChild(renderer.domElement);
    const controls = new THREE.OrbitControls(camera, renderer.domElement);

    const box_size = 80;
    const geometry_box = new THREE.BoxGeometry(box_size, box_size, box_size);

    // Draw boxes.
    for (var i = 0; i < coordinates_box.length; i++) {
        c = coordinates_box[i]
        const material_box = new THREE.MeshStandardMaterial({color: c[3]});
        const box = new THREE.Mesh(geometry_box, material_box);
        box.castShadow = true;
        box.receiveShadow = true;
        box.position.set(c[0], c[1], c[2]);
        scene.add(box);
    }

    // Draw grid lines.
    var material_line = new THREE.LineBasicMaterial({
        color: 0xffffff,
        transparent: true,
        opacity: 0.2
    });

    for (var i = 0; i < coordinates_line.length; i++) {
        var geometry_line = new THREE.Geometry();
        c0 = coordinates_line[i][0]
        c1 = coordinates_line[i][1]
        geometry_line.vertices.push(
            new THREE.Vector3(c0[0], c0[1], c0[2]),
            new THREE.Vector3(c1[0], c1[1], c1[2])
        );
        var line = new THREE.Line(geometry_line, material_line);
        scene.add(line);
    }

    const light1 = new THREE.PointLight();
    const light2 = new THREE.PointLight();
    //light1.castShadow = true;
    //light2.castShadow = true;
    light1.intensity = 2;
    light2.intensity = 2;
    light1.position.set(0, 10000, 0);
    light2.position.set(-10000, -10000, -10000);
    scene.add(light1);
    scene.add(light2);

    var slider = document.getElementById("slider");
    slider.addEventListener("input", changeIntensity);

    function changeIntensity(e){
        light1.intensity = e.target.value;
    }

    tick();

    function tick() {
        renderer.render(scene, camera);
        requestAnimationFrame(tick);
    }
}
