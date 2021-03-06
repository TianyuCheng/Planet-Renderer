#include <Camera.h>

Camera::Camera(QString n) : name(n), dirty(true) {
    // Initialize the matrices
    uMVMatrix.setToIdentity();
    uPMatrix.setToIdentity();
    uNMatrix.setToIdentity();
}

Camera::~Camera() {
}

void Camera::initialize() {
}

void Camera::setCenter(QVector3D l) {
    this->lookAt(eye, l, up);
}

void Camera::setEye(QVector3D e) {
    this->lookAt(e, center, up);
}

void Camera::setUp(QVector3D u) {
    this->lookAt(eye, center, u);
}

void Camera::lookAt(QVector3D eye, QVector3D center, QVector3D up) {
    this->eye = eye;
    this->center = center;
    this->up = up;
    this->look = center - eye;

    this->up.normalize();

    uMVMatrix.setToIdentity();
    uMVMatrix.lookAt(eye, center, up);

    dirty = true;

    // modelviewInfo();
}

void Camera::moveForward(double distance) {
    QVector3D diff = look.normalized() * distance;
    eye += diff;
    center += diff;
    uMVMatrix.setToIdentity();
    uMVMatrix.lookAt(eye, center, up);

    dirty = true;
    // modelviewInfo();
}

void Camera::move(QVector3D daxes)
{
	QVector3D X(QVector3D::crossProduct(look, up).normalized());
	QVector3D Y(up.normalized());
	QVector3D Z(-look.normalized());

	QVector3D deye;
	// Should be matrix but I'm too laze to reason the formula.
	deye += daxes.x() * X;
	deye += daxes.y() * Y;
	deye += daxes.z() * Z;

	setEye(getPosition() + deye);
}

void Camera::moveBackward(double distance) {
    moveForward(-distance);
}

void Camera::turnLeft(double angle, QVector3D axis) {
    QQuaternion q = QQuaternion::fromAxisAndAngle(axis, angle);
    look = q.rotatedVector(look);
    up = q.rotatedVector(up);
    center = eye + look;
    uMVMatrix.setToIdentity();
    uMVMatrix.lookAt(eye, center, up);
    
    dirty = true;
    // modelviewInfo();
}

void Camera::turnRight(double angle, QVector3D axis) {
    turnLeft(-angle, axis);
}

void Camera::lookUp(double angle) {
    QVector3D axis = QVector3D::crossProduct(look, up).normalized();
    QQuaternion q = QQuaternion::fromAxisAndAngle(axis, angle);

    // We have not moved our eye, only updated our view
    up = q.rotatedVector(up);
    look = q.rotatedVector(look);
    center = eye + look;

    uMVMatrix.setToIdentity();
    uMVMatrix.lookAt(eye, center, up);
    dirty = true;
}

void Camera::lookDown(double angle) {
    lookUp(-angle);
}

void Camera::setLeft(double left) { 
    this->left = left; 
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    dirty = true;
}

void Camera::setRight(double right) { 
    this->right = right; 
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    dirty = true;
}

void Camera::setbottom(double bottom) { 
    this->bottom = bottom; 
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    dirty = true;
}

void Camera::setTop(double top) {
    this->top = top; 
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    dirty = true;
}

void Camera::setNear(double near) { 
    this->near = near; 
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    dirty = true;
}

void Camera::setFar(double far) { 
    this->far = far; 
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    dirty = true;
}

void Camera::setFrustum(double left,    double right,
                double bottom,  double top,
                double near,    double far) {
    this->left = left; 
    this->right = right;
    this->bottom = bottom; 
    this->top = top;
    this->near = near; 
    this->far = far;
    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far);
    dirty = true;
}

void Camera::setFOV(double fov) {
    setPerspective(
            fov, 
            double((right - left) / (top - bottom)), 
            near, far);
}

void Camera::setAspect(double aspect) {
    setPerspective(
            2 * qAtan(0.5 * (top - bottom) / near),
            aspect,
            near, far);
}

void Camera::setPerspective(double fovy, double aspect, double zNear, double zFar) {

    double halfHeight = zNear * qTan(qDegreesToRadians(fovy * 0.5));
    double halfWidth = halfHeight * aspect;

    left = -halfWidth;
    right = halfWidth;
    bottom = -halfHeight;
    top = +halfHeight;
    near = zNear;
    far = zFar;

    uPMatrix.setToIdentity();
    uPMatrix.frustum(left, right, bottom, top, near, far); 
    
    dirty = true;
    // projectionInfo();
}

void Camera::reflectCamera(QVector4D n, Camera *cam) {
    // Find a point on the plane
    QVector3D p;
    double a = n.x();
    double b = n.y();
    double c = n.z();
    double d = n.w();
    if (n.x() != 0) { p = QVector3D(-d / a, 0.0, 0.0); }
    else if (n.y() != 0) { p = QVector3D(0.0, -d / b, 0.0); }
    else if (n.z() != 0) { p = QVector3D(0.0, 0.0, -d / c); }

    QVector3D normalizedDir = look.normalized();
    QVector3D baseX = QVector3D::crossProduct(normalizedDir, up).normalized();

    QVector3D planeNormal = QVector3D(a, b, c).normalized();
    QVector3D normal;

    // reflect camera eye
    double dist = QVector3D::dotProduct((eye - p), planeNormal);
    if (dist >= 0) normal = -planeNormal;
    else normal = planeNormal;
    dist = qAbs(dist);
    QVector3D newEye = eye + normal * 2 * dist;

    // reflect camera center
    dist = QVector3D::dotProduct((center - p), planeNormal);
    if (dist >= 0) normal = -planeNormal;
    else normal = planeNormal;
    dist = qAbs(dist);
    QVector3D newCenter = center + normal * 2 * dist;

    // find out the new up vector
    QVector3D newLook = newCenter - newEye;
    QVector3D newUp = QVector3D::crossProduct(baseX, newLook).normalized();
    cam->lookAt(newEye, newCenter, newUp);
}

void Camera::uniformMatrices(QOpenGLShaderProgram &program) {
    // uniform matrices
    program.setUniformValue("uMVMatrix", uMVMatrix);
    program.setUniformValue("uPMatrix", uPMatrix);
    program.setUniformValue("uNMatrix", uNMatrix);
}

Camera::Cullable Camera::isCullable(BoundingBox box) {

    // If the frustum has been changed, update it
    if (dirty) updateFrustum();

    QVector3D* corners = box.getCorners();

    int total = 0;

    // Iterate through every plane
    for (int i = 0; i < 6; i++) {
        int count = 8;
        int point = 1;

        QPair<QVector3D, QVector3D> plane = frustum[i];

        // Iterate through every corner
        for (int j = 0; j < 8; j++) {
            QVector3D corner = corners[j];
            if (QVector3D::dotProduct(plane.first, corner - plane.second) > 0) {  // out
                count--;
                point = 0;
            }
        }

        // We are out
        if (count == 0) return Camera::Cullable::TOTALLY_CULLABLE;
        total += point;
    }

    if (total == 8) return Camera::Cullable::NOT_CULLABLE;

    return Camera::Cullable::PARTIALLY_CULLABLE;
}


void Camera::updateFrustum() {
    QVector3D corners[8];

    QVector3D normalizedDir = look.normalized();
    QVector3D center = eye + normalizedDir * near;

    QVector3D baseX = QVector3D::crossProduct(normalizedDir, up).normalized();
    QVector3D baseY = up;

    /**
     * 1------0
     * |      | corners in the near plane
     * 2------3
     * */
    corners[0] = center + 0.5 * (right - left) * baseX + 0.5 * (top - bottom) * baseY;
    corners[1] = center - 0.5 * (right - left) * baseX + 0.5 * (top - bottom) * baseY;
    corners[2] = center - 0.5 * (right - left) * baseX - 0.5 * (top - bottom) * baseY;
    corners[3] = center + 0.5 * (right - left) * baseX - 0.5 * (top - bottom) * baseY;

    center = eye + normalizedDir * far;
    baseX *= far / near;
    baseY *= far / near;

    /**
     * 5------4
     * |      | corners in the far plane
     * 6------7
     * */
    corners[4] = center + 0.5 * (right - left) * baseX + 0.5 * (top - bottom) * baseY;
    corners[5] = center - 0.5 * (right - left) * baseX + 0.5 * (top - bottom) * baseY;
    corners[6] = center - 0.5 * (right - left) * baseX - 0.5 * (top - bottom) * baseY;
    corners[7] = center + 0.5 * (right - left) * baseX - 0.5 * (top - bottom) * baseY;

    frustum.clear();

    // near plane and far plane
    frustum << qMakePair(-normalizedDir, corners[0]);
    frustum << qMakePair(normalizedDir, corners[4]);

    // left plane and right plane
    QVector3D leftNormal = QVector3D::crossProduct(corners[5] - corners[1], corners[2] - corners[1]);
    QVector3D rightNormal = QVector3D::crossProduct(corners[7] - corners[3], corners[0] - corners[3]);
    frustum << qMakePair(leftNormal, corners[1]);
    frustum << qMakePair(rightNormal, corners[0]);

    // top plane and bottom plane
    QVector3D topNormal = QVector3D::crossProduct(corners[4] - corners[0], corners[1] - corners[0]);
    QVector3D bottomNormal = QVector3D::crossProduct(corners[6] - corners[2], corners[3] - corners[2]);
    frustum << qMakePair(topNormal, corners[0]);
    frustum << qMakePair(bottomNormal, corners[2]);

    dirty = false;
}
