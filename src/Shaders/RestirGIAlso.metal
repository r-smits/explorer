
#define ASPECT_RATIO (iResolution.x / iResolution.y)
#define STRATEGY 1

float intensity(vec3 c) {
	return (c.r + c.g + c.b) / 3.0;
}

// Transform UVs from [0..1] to [-1..1] space
vec2 normalizeUVs(vec2 uvs) {
	return uvs * 2.0 - 1.0;
}

vec3 getCameraPos(float time) {
	mat3 rot = mat3(
		vec3(cos(time), 0.0, -sin(time)),
		vec3(0.0,       1.0,  0.0),
		vec3(sin(time), 0.0,  cos(time)));
	return rot * vec3(0.0, 100.0, 1000.0);
}

vec3 getViewDirection(vec3 cameraPos) {
	return normalize(vec3(0.0) - cameraPos);
}

vec3 getLightDir(float time) {
	return normalize(vec3(1.0));
}

bool intersectPlane(vec3 n, vec3 p0, vec3 l0, vec3 l, out float t) { 
	// assuming vectors are all normalized
	float denom = dot(n, l); 
	if (denom < 0.0) {
		denom = -denom;
		n = -n;
	}
	if (denom > 1e-6) { 
		vec3 p0l0 = p0 - l0; 
		t = dot(p0l0, n) / denom; 
		return (t >= 0.0); 
	}
	return false; 
} 

struct Rect {
	vec3 center;
	mat3 basis;
	vec2 scale;
};

bool intersectRect(vec3 rayOrigin, vec3 rayDir, Rect r, out vec3 p, out vec3 n) {
	float t;
	if (!intersectPlane(r.basis[2], r.center, rayOrigin, rayDir, t))
		return false;
	
	if (dot(rayDir, r.basis[2]) < 0.0) {
		n = r.basis[2];
	} else {
		n = -r.basis[2];
	}
	p = rayOrigin + t * rayDir;
	vec3 centerToIntersection = p - r.center;
	return 
		(abs(dot(centerToIntersection, r.basis[0])) < r.scale.x) && 
		(abs(dot(centerToIntersection, r.basis[1])) < r.scale.y);
}

struct Scene {
	Rect ground;
	Rect light;
};

void initScene(out Scene scene) {
	scene.ground.center = vec3(0.0, -100, 0);
	scene.ground.basis = mat3(
		vec3(1, 0, 0),
		vec3(0, 0, 1),
		vec3(0, 1, 0)
	);
	scene.ground.scale = vec2(1000.0, 1000.0);

	scene.light.center = vec3(0.0, 200, 0);
	scene.light.basis = mat3(
		vec3(1, 0, 0),
		vec3(0, 0, 1),
		vec3(0, -1, 0)
	);
	scene.light.scale = vec2(100.0, 100.0);
}

bool intersectScene(Scene scene, vec3 rayOrigin, vec3 rayDir, out vec3 p, out vec3 n, out int material) {
	if (intersectRect(rayOrigin, rayDir, scene.ground, p, n)) {
		material = 0;
		return true;
	}

	if (intersectRect(rayOrigin, rayDir, scene.light, p, n)) {
		material = 1;
		return true;
	}

	return false;
}

// return random number in [0, 1)
float hashRand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void sampleRectLight(
	Rect r,
	vec3 shadowPoint,
	vec2 uv,
	out vec3 color,
	out vec3 lightPos,
	out vec3 lightDir
) {
	uv = (uv - 0.5) * 2.0 * r.scale;
	lightPos = r.center + r.basis[0] * uv.x + r.basis[1] * uv.y;
	lightDir = lightPos - shadowPoint;
	float dist = length(lightDir);
	lightDir /= dist;
	color = vec3(300000.0) / (dist * dist);
}

struct Reservoir {
	vec2 y;
	float sumw;
	int m;
};

void initReservoir(inout Reservoir r) {
	r.y = vec2(0.0);
	r.sumw = 0.0;
	r.m = 0;
}

void updateReservoir(inout Reservoir r, vec2 x, float w, int seed) {
	r.sumw += w;
	r.m++;
	float u = hashRand(gl_FragCoord.xy + vec2(0.01, 0.023) + float(seed));
	if (u < (w / r.sumw)) {
		r.y = x;
	}
}

vec4 encodeReservoir(Reservoir r) {
	return vec4(r.y, r.sumw, float(r.m));
}

vec3 unshadowedContribution(Scene scene, vec3 shadowPoint, vec3 n, vec2 uv) {
	vec3 lightColor, lightPos, lightDir;
	sampleRectLight(scene.light, shadowPoint, uv, lightColor, lightPos, lightDir);
	float NoL = max(0.0, dot(n, lightDir));
	return vec3(0.5) * NoL * lightColor;
}

float risTarget(Scene scene, vec3 shadowPoint, vec3 n, vec2 uv) {
	return intensity(unshadowedContribution(scene, shadowPoint, n, uv));
}

Reservoir resampledImportanceSampling(Scene scene, vec3 shadowPoint, vec3 n, int sampleIdx) {
	const int M = 4;

	Reservoir r;
	initReservoir(r);

	const float risProb = 1.0; // uniform sampling
	float u = hashRand(shadowPoint.xy + vec2(0.01, 0.023) + vec2(sampleIdx));
	float v = hashRand(shadowPoint.xy + vec2(0.04, 0.056) + vec2(sampleIdx));
	float delta = 1.0 / float(M);
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < M; j++) {
			vec2 uv = vec2(u + float(i), v + float(j)) * delta;
			float w = risTarget(scene, shadowPoint, n, uv) / risProb;
			updateReservoir(r, uv, w, j * M + i);	
		}
	}

	return r;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	vec2  offset      = normalizeUVs(fragCoord.xy / iResolution.xy);
	vec3  cameraPos   = getCameraPos(0.0);
	vec3  cameraView  = getViewDirection(cameraPos);
	vec3  cameraUp    = vec3(0.0, 1.0, 0.0);
	vec3  cameraRight = cross(cameraView, cameraUp);
	float cameraNear  = 4.0;
	
	vec3 ray = normalize(cameraRight * offset.x * ASPECT_RATIO + cameraUp * offset.y + cameraView * cameraNear);

	Scene scene;
	initScene(scene);
	vec3 p, n;
	int material;
	if (intersectScene(scene, cameraPos, ray, p, n, material)) {
		if (material == 0) {
			Reservoir r = resampledImportanceSampling(scene, p, n, 0);
			fragColor = encodeReservoir(r);
		} else {
			fragColor = vec4(0.0);
		}		
	} else {
		fragColor = vec4(0.0);
	}
}

