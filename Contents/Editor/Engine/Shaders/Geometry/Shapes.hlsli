#ifndef GEOMETRY_SHAPES_H
#define GEOMETRY_SHAPES_H

struct Sphere
{
	float3 Center;
	float Radius;
};

struct Cone
{
	float3 Tip;
	float Height;
	float3 Distanec;
	float Radius;
};

struct Cylinder
{
	float3 Center;
	float Height;
	float3 Distance;
	float Radius;
};

struct Plane
{
	float3 Normal;
	float Distance;
};

struct Box
{
	float3 Center;
	float3 Extents;
};

//

Plane PlaneFromPoints(const in float3 p0, const in float3 p1, const in float3 p2)
{
	Plane plane;
	plane.Normal = normalize(cross(p1 - p0, p2 - p0));
	plane.Distance = dot(plane.Normal, p0);
	return plane;
}

//

bool SphereInsidePlane(const in Sphere sphere, const in Plane plane)
{
	return dot(plane.Normal, sphere.Center) - plane.Distance < sphere.Radius;
}

bool SphereInsideSphere(const in Sphere sphere0, const in Sphere sphere1)
{
	return length(sphere0.Center - sphere1.Center) < sphere0.Radius + sphere1.Radius;
}

bool SphereInsideCone(const in Sphere sphere, const in Cone cone)
{
	float3 v = sphere.Center - cone.Tip;
	float3 n = normalize(cone.Distanec);
	float d = dot(v, n);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - n * d) < r + sphere.Radius;
}

bool SphereInsideCylinder(const in Sphere sphere, const in Cylinder cylinder)
{
	float3 v = sphere.Center - cylinder.Center;
	float3 n = normalize(cylinder.Distance);
	float d = dot(v, n);
	float r = cylinder.Radius * (cylinder.Height - d) / cylinder.Height;
	return length(v - n * d) < r + sphere.Radius;
}

bool SphereInsideBox(const in Sphere sphere, const in Box box)
{
	float3 v = abs(sphere.Center - box.Center);
	float3 e = box.Extents;
	return v.x < e.x + sphere.Radius && v.y < e.y + sphere.Radius && v.z < e.z + sphere.Radius;
}

//

bool ConeInsidePlane(const in Cone cone, const in Plane plane)
{
	return dot(plane.Normal, cone.Tip) - plane.Distance < cone.Radius;
}

bool ConeInsideSphere(const in Cone cone, const in Sphere sphere)
{
	float3 v = sphere.Center - cone.Tip;
	float3 n = normalize(cone.Distanec);
	float d = dot(v, n);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - n * d) < r + sphere.Radius;
}

bool ConeInsideCone(const in Cone cone0, const in Cone cone1)
{
	float3 v = cone1.Tip - cone0.Tip;
	float3 n = normalize(cone0.Distanec);
	float d = dot(v, n);
	float r = cone0.Radius * (cone0.Height - d) / cone0.Height;
	return length(v - n * d) < r + cone1.Radius;
}

bool ConeInsideCylinder(const in Cone cone, const in Cylinder cylinder)
{
	float3 v = cylinder.Center - cone.Tip;
	float3 n = normalize(cone.Distanec);
	float d = dot(v, n);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - n * d) < r + cylinder.Radius;
}

bool ConeInsideBox(const in Cone cone, const in Box box)
{
	float3 v = abs(box.Center - cone.Tip);
	float3 e = box.Extents;
	float3 n = normalize(cone.Distanec);
	float d = dot(v, n);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - n * d) < r + length(e);
}

//

bool CylinderInsidePlane(const in Cylinder cylinder, const in Plane plane)
{
	return dot(plane.Normal, cylinder.Center) - plane.Distance < cylinder.Radius;
}

bool CylinderInsideSphere(const in Cylinder cylinder, const in Sphere sphere)
{
	float3 v = sphere.Center - cylinder.Center;
	float3 n = normalize(cylinder.Distance);
	float d = dot(v, n);
	float r = cylinder.Radius * (cylinder.Height - d) / cylinder.Height;
	return length(v - n * d) < r + sphere.Radius;
}

bool CylinderInsideCone(const in Cylinder cylinder, const in Cone cone)
{
	float3 v = cone.Tip - cylinder.Center;
	float3 n = normalize(cylinder.Distance);
	float d = dot(v, n);
	float r = cylinder.Radius * (cylinder.Height - d) / cylinder.Height;
	return length(v - n * d) < r + cone.Radius;
}

bool CylinderInsideCylinder(const in Cylinder cylinder0, const in Cylinder cylinder1)
{
	float3 v = cylinder1.Center - cylinder0.Center;
	float3 n = normalize(cylinder0.Distance);
	float d = dot(v, n);
	float r = cylinder0.Radius * (cylinder0.Height - d) / cylinder0.Height;
	return length(v - n * d) < r + cylinder1.Radius;
}

bool CylinderInsideBox(const in Cylinder cylinder, const in Box box)
{
	float3 v = abs(box.Center - cylinder.Center);
	float3 e = box.Extents;
	float3 n = normalize(cylinder.Distance);
	float d = dot(v, n);
	float r = cylinder.Radius * (cylinder.Height - d) / cylinder.Height;
	return length(v - n * d) < r + length(e);
}

//

bool BoxInsidePlane(const in Box box, const in Plane plane)
{
	float3 e = box.Extents;
	float3 n = plane.Normal;
	float d = plane.Distance;
	float r = e.x * abs(n.x) + e.y * abs(n.y) + e.z * abs(n.z);
	return dot(n, box.Center) - d < r;
}

bool BoxInsideSphere(const in Box box, const in Sphere sphere)
{
	float3 v = abs(sphere.Center - box.Center);
	float3 e = box.Extents;
	return v.x < e.x + sphere.Radius && v.y < e.y + sphere.Radius && v.z < e.z + sphere.Radius;
}

bool BoxInsideCone(const in Box box, const in Cone cone)
{
	float3 v = abs(cone.Tip - box.Center);
	float3 e = box.Extents;
	float3 n = normalize(cone.Distanec);
	float d = dot(v, n);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - n * d) < r + length(e);
}

bool BoxInsideCylinder(const in Box box, const in Cylinder cylinder)
{
	float3 v = abs(cylinder.Center - box.Center);
	float3 e = box.Extents;
	float3 n = normalize(cylinder.Distance);
	float d = dot(v, n);
	float r = cylinder.Radius * (cylinder.Height - d) / cylinder.Height;
	return length(v - n * d) < r + length(e);
}

bool BoxInsideBox(const in Box box0, const in Box box1)
{
	float3 v = abs(box1.Center - box0.Center);
	float3 e0 = box0.Extents;
	float3 e1 = box1.Extents;
	return v.x < e0.x + e1.x && v.y < e0.y + e1.y && v.z < e0.z + e1.z;
}

//

bool PlaneInsidePlane(const in Plane plane0, const in Plane plane1)
{
	return dot(plane0.Normal, plane1.Normal) - plane0.Distance < plane1.Distance;
}

bool PlaneInsideSphere(const in Plane plane, const in Sphere sphere)
{
	return SphereInsidePlane(sphere, plane);
}

bool PlaneInsideCone(const in Plane plane, const in Cone cone)
{
	return ConeInsidePlane(cone, plane);
}

bool PlaneInsideCylinder(const in Plane plane, const in Cylinder cylinder)
{
	return CylinderInsidePlane(cylinder, plane);
}

bool PlaneInsideBox(const in Plane plane, const in Box box)
{
	return BoxInsidePlane(box, plane);
}


#endif