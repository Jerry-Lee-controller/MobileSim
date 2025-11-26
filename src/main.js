const canvas = document.getElementById('sim');
const ctx = canvas.getContext('2d');

const hud = {
  speed: document.getElementById('speed'),
  altitude: document.getElementById('altitude'),
  pitch: document.getElementById('pitch'),
  fuel: document.getElementById('fuel'),
  score: document.getElementById('score'),
  state: document.getElementById('state'),
};

const inputState = {
  throttle: 0,
  yaw: 0,
  pitch: 0,
};

const plane = {
  x: 0,
  y: 0,
  vx: 0,
  vy: 0,
  heading: 0,
  pitch: 5,
  altitude: 20,
  throttle: 0.2,
  fuel: 1,
  score: 0,
};

const world = {
  gravity: 9.81,
  liftFactor: 0.12,
  drag: 0.0025,
  thrustPower: 18,
  maxAltitude: 200,
  rings: [],
};

let lastTime = 0;

function resize() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
}
window.addEventListener('resize', resize);
resize();

function clamp(v, min, max) {
  return Math.min(max, Math.max(min, v));
}

function spawnRing() {
  const distance = 500 + Math.random() * 600;
  const angle = plane.heading + (Math.random() - 0.5) * 1.2;
  world.rings.push({
    x: plane.x + Math.cos(angle) * distance,
    y: plane.y + Math.sin(angle) * distance,
    altitude: 30 + Math.random() * 120,
    radius: 40 + Math.random() * 30,
    passed: false,
  });
  if (world.rings.length > 6) world.rings.shift();
}

for (let i = 0; i < 4; i++) spawnRing();

function update(dt) {
  plane.throttle = clamp(plane.throttle + inputState.throttle * dt * 0.25, 0, 1);
  plane.heading += inputState.yaw * dt * 1.4;
  plane.pitch = clamp(plane.pitch + inputState.pitch * dt * 35, -20, 35);

  const speed = Math.hypot(plane.vx, plane.vy);
  const thrust = plane.throttle * world.thrustPower;
  const drag = world.drag * speed * speed;
  const lift = clamp(speed * world.liftFactor, 0, 25) * (1 + plane.pitch / 40);
  const climb = (lift - world.gravity) * dt;

  plane.altitude = clamp(plane.altitude + climb, 0, world.maxAltitude);

  const angle = plane.heading;
  const ax = Math.cos(angle) * thrust - Math.cos(angle) * drag;
  const ay = Math.sin(angle) * thrust - Math.sin(angle) * drag;

  plane.vx += ax * dt;
  plane.vy += ay * dt;

  plane.vx *= 0.999;
  plane.vy *= 0.999;

  plane.x += plane.vx * dt;
  plane.y += plane.vy * dt;

  plane.fuel = clamp(plane.fuel - plane.throttle * dt * 0.02, 0, 1);

  world.rings.forEach((ring) => {
    const dx = ring.x - plane.x;
    const dy = ring.y - plane.y;
    const dist = Math.hypot(dx, dy);
    const inAltitude = Math.abs(ring.altitude - plane.altitude) < 15;
    if (!ring.passed && dist < ring.radius && inAltitude) {
      ring.passed = true;
      plane.score += 100;
      spawnRing();
    }
  });

  hud.speed.textContent = (speed * 3.6).toFixed(0);
  hud.altitude.textContent = plane.altitude.toFixed(0);
  hud.pitch.textContent = plane.pitch.toFixed(0);
  hud.fuel.textContent = Math.round(plane.fuel * 100);
  hud.score.textContent = plane.score.toLocaleString();
  hud.state.textContent = plane.altitude < 2 ? 'Taxi' : plane.altitude < 15 ? 'Climb' : 'Cruise';
}

function drawBackground() {
  ctx.fillStyle = '#04070f';
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  const gradient = ctx.createRadialGradient(
    canvas.width * 0.6,
    canvas.height * 0.4,
    120,
    canvas.width * 0.5,
    canvas.height * 0.6,
    Math.max(canvas.width, canvas.height)
  );
  gradient.addColorStop(0, '#0a1627');
  gradient.addColorStop(1, '#02050d');
  ctx.fillStyle = gradient;
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  ctx.fillStyle = 'rgba(255,255,255,0.25)';
  for (let i = 0; i < 60; i++) {
    const x = (Math.random() * canvas.width) | 0;
    const y = (Math.random() * canvas.height) | 0;
    const size = Math.random() * 2;
    ctx.fillRect(x, y, size, size);
  }
}

function drawPlane() {
  const viewX = canvas.width / 2;
  const viewY = canvas.height / 2;

  ctx.save();
  ctx.translate(viewX, viewY);
  ctx.rotate(plane.heading);

  ctx.fillStyle = '#4ad6ff';
  ctx.strokeStyle = '#82ffe6';
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(28, 0);
  ctx.lineTo(-22, 12);
  ctx.lineTo(-10, 0);
  ctx.lineTo(-22, -12);
  ctx.closePath();
  ctx.fill();
  ctx.stroke();

  ctx.restore();

  ctx.fillStyle = 'rgba(130,255,230,0.5)';
  ctx.fillRect(viewX - 1.5, viewY - 1.5, 3, 3);
}

function worldToScreen(x, y) {
  const viewX = canvas.width / 2 - (plane.x - x) * 0.4;
  const viewY = canvas.height / 2 - (plane.y - y) * 0.4;
  return { x: viewX, y: viewY };
}

function drawRings() {
  world.rings.forEach((ring) => {
    const { x, y } = worldToScreen(ring.x, ring.y);
    const alpha = ring.passed ? 0.35 : 0.9;
    ctx.strokeStyle = `rgba(255, 203, 107, ${alpha})`;
    ctx.lineWidth = 4;
    ctx.beginPath();
    ctx.arc(x, y, ring.radius, 0, Math.PI * 2);
    ctx.stroke();

    ctx.fillStyle = `rgba(255, 255, 255, ${alpha})`;
    ctx.font = '12px Inter';
    ctx.textAlign = 'center';
    ctx.fillText(`${ring.altitude.toFixed(0)} m`, x, y - ring.radius - 8);
  });
}

function drawHorizon() {
  const horizonY = canvas.height / 2 + (plane.pitch * 2);
  ctx.strokeStyle = 'rgba(74, 214, 255, 0.4)';
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(0, horizonY);
  ctx.lineTo(canvas.width, horizonY);
  ctx.stroke();
}

function render() {
  drawBackground();
  drawRings();
  drawPlane();
  drawHorizon();
}

function loop(timestamp) {
  const dt = Math.min(0.05, (timestamp - lastTime) / 1000 || 0);
  lastTime = timestamp;
  update(dt);
  render();
  requestAnimationFrame(loop);
}
requestAnimationFrame(loop);

function bindControls() {
  const keys = new Set();
  const updateFromKeys = () => {
    inputState.throttle = keys.has('ArrowUp') ? 1 : keys.has('ArrowDown') ? -1 : 0;
    inputState.yaw = keys.has('ArrowLeft') ? -1 : keys.has('ArrowRight') ? 1 : 0;
    inputState.pitch = keys.has('KeyW') ? 1 : keys.has('KeyS') ? -1 : 0;
  };

  window.addEventListener('keydown', (e) => {
    keys.add(e.code);
    updateFromKeys();
  });
  window.addEventListener('keyup', (e) => {
    keys.delete(e.code);
    updateFromKeys();
  });

  document.querySelectorAll('.control').forEach((btn) => {
    const action = btn.dataset.action;
    const start = () => {
      switch (action) {
        case 'throttle-up':
          inputState.throttle = 1;
          break;
        case 'throttle-down':
          inputState.throttle = -1;
          break;
        case 'yaw-left':
          inputState.yaw = -1;
          break;
        case 'yaw-right':
          inputState.yaw = 1;
          break;
        case 'pitch-up':
          inputState.pitch = 1;
          break;
        case 'pitch-down':
          inputState.pitch = -1;
          break;
      }
    };
    const stop = () => {
      if (action.startsWith('throttle')) inputState.throttle = 0;
      if (action.startsWith('yaw')) inputState.yaw = 0;
      if (action.startsWith('pitch')) inputState.pitch = 0;
    };
    btn.addEventListener('pointerdown', start);
    btn.addEventListener('pointerup', stop);
    btn.addEventListener('pointerleave', stop);
    btn.addEventListener('touchstart', (e) => {
      e.preventDefault();
      start();
    }, { passive: false });
    btn.addEventListener('touchend', (e) => {
      e.preventDefault();
      stop();
    }, { passive: false });
  });
}

bindControls();
