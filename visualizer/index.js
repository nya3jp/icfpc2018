// Documents
// - https://ics.media/tutorial-three/index.html
//   - Nice introduction for Three.js
// - https://ics.media/entry/14771
//   - Similar to above
// - https://jsfiddle.net/prisoner849/vo1urg68/
//   - Three.js example with slider

window.addEventListener('DOMContentLoaded', init);

function init() {
    const width = 960;
    const height = 540;

    const renderer = new THREE.WebGLRenderer({
        canvas: document.querySelector('#myCanvas')
    });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(width, height);

    const scene = new THREE.Scene();

    const camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);
    camera.position.set(3000, 3000, 3000);

    document.body.appendChild(renderer.domElement);
    const controls = new THREE.OrbitControls(camera, renderer.domElement);

    const geometry = new THREE.BoxGeometry(50, 50, 50);
    const material = new THREE.MeshStandardMaterial({color: 0x0000FF});

    for (var i = 0; i < coordinates.length; i++) {
        const box = new THREE.Mesh(geometry, material);
        box.position.set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
        scene.add(box);
    }

    const light1 = new THREE.DirectionalLight(0xFFFFFF);
    const light2 = new THREE.DirectionalLight(0xFFFFFF);
    light1.intensity = 2;
    light2.intensity = 2;
    light1.position.set(1, 1, 1);
    light2.position.set(-1, -1, -1);
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
