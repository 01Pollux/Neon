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
	float3 Direction;
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

bool PointInsideSphere(const in Sphere sphere, const in float3 pt)
{
	return length(pt - sphere.Center) < sphere.Radius;
}

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
	float d = dot(v, cone.Direction);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - cone.Direction * d) < r + sphere.Radius;
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

bool PointInsideCone(const in Cone cone, const in float3 pt)
{
	float3 v = pt - cone.Tip;
	float d = dot(v, cone.Direction);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - cone.Direction * d) < r;
}

bool ConeInsidePlane(const in Cone cone, const in Plane plane)
{
	return dot(plane.Normal, cone.Tip) - plane.Distance < cone.Radius;
}

bool ConeInsideSphere(const in Cone cone, const in Sphere sphere)
{
	return SphereInsideCone(sphere, cone);
}

bool ConeInsideCone(const in Cone cone0, const in Cone cone1)
{
	float3 v = cone1.Tip - cone0.Tip;
	float d = dot(v, cone0.Direction);
	float r = cone0.Radius * (cone0.Height - d) / cone0.Height;
	return length(v - cone0.Direction * d) < r + cone1.Radius;
}

bool ConeInsideCylinder(const in Cone cone, const in Cylinder cylinder)
{
	float3 v = cylinder.Center - cone.Tip;
	float d = dot(v, cone.Direction);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - cone.Direction * d) < r + cylinder.Radius;
}

bool ConeInsideBox(const in Cone cone, const in Box box)
{
	float3 v = abs(box.Center - cone.Tip);
	float3 e = box.Extents;
	float d = dot(v, cone.Direction);
	float r = cone.Radius * (cone.Height - d) / cone.Height;
	return length(v - cone.Direction * d) < r + length(e);
}

//

bool PointInsideCylinder(const in Cylinder cylinder, const in float3 pt)
{
	float3 v = pt - cylinder.Center;
	float3 n = normalize(cylinder.Distance);
	float d = dot(v, n);
	float r = cylinder.Radius * (cylinder.Height - d) / cylinder.Height;
	return length(v - n * d) < r;
}

bool CylinderInsidePlane(const in Cylinder cylinder, const in Plane plane)
{
	return dot(plane.Normal, cylinder.Center) - plane.Distance < cylinder.Radius;
}

bool CylinderInsideSphere(const in Cylinder cylinder, const in Sphere sphere)
{
	return SphereInsideCylinder(sphere, cylinder);
}

bool CylinderInsideCone(const in Cylinder cylinder, const in Cone cone)
{
	return ConeInsideCylinder(cone, cylinder);
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

bool PointInsideBox(const in Box box, const in float3 pt)
{
	float3 v = abs(pt - box.Center);
	float3 e = box.Extents;
	return v.x < e.x && v.y < e.y && v.z < e.z;
}

bool BoxInsidePlane(const in Box box, const in Plane plane)
{
	return dot(plane.Normal, box.Center) - plane.Distance < length(box.Extents);
}

bool BoxInsideSphere(const in Box box, const in Sphere sphere)
{
	return SphereInsideBox(sphere, box);
}

bool BoxInsideCone(const in Box box, const in Cone cone)
{
	return ConeInsideBox(cone, box);
}

bool BoxInsideCylinder(const in Box box, const in Cylinder cylinder)
{
	return CylinderInsideBox(cylinder, box);
}

bool BoxInsideBox(const in Box box0, const in Box box1)
{
	float3 v = abs(box1.Center - box0.Center);
	float3 e = box0.Extents + box1.Extents;
	return v.x < e.x && v.y < e.y && v.z < e.z;
}

//

bool PointInsidePlane(const in Plane plane0, const in float3 pt)
{
	return dot(plane0.Normal, pt) - plane0.Distance < length(plane0.Normal);
}

bool PlaneInsidePlane(const in Plane plane0, const in Plane plane1)
{
	return dot(plane0.Normal, plane1.Normal) - plane0.Distance < length(plane1.Normal);
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