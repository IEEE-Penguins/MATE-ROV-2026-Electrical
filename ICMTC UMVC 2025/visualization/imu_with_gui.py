import json
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import serial

# ====== Serial Config ======
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200

# ====== ROV Orientation ======
angle_roll = 0.0
angle_pitch = 0.0

# ====== Serial Connection ======
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"[ERROR] Serial not connected: {e}")
    ser = None

# ====== Draw Axes ======
def draw_axes(length=2.0):
    glBegin(GL_LINES)
    # X-axis (Red)
    glColor3f(1, 0, 0)
    glVertex3f(0, 0, 0)
    glVertex3f(length, 0, 0)
    # Y-axis (Green)
    glColor3f(0, 1, 0)
    glVertex3f(0, 0, 0)
    glVertex3f(0, length, 0)
    # Z-axis (Blue)
    glColor3f(0, 0, 1)
    glVertex3f(0, 0, 0)
    glVertex3f(0, 0, length)
    glEnd()

# ====== Draw Cube ======
def draw_cube():
    glColor4f(0.6, 0.7, 1.0, 1.0)
    glutSolidCube(1.0)

# ====== Display Scene ======
def display():
    global angle_roll, angle_pitch

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -5.0)

    # Apply rotation from sensor
    glRotatef(angle_pitch, 1.0, 0.0, 0.0)
    glRotatef(angle_roll, 0.0, 0.0, 1.0)

    draw_axes()
    draw_cube()

    glutSwapBuffers()

# ====== Update Sensor Data from Serial ======
def update_data():
    global angle_roll, angle_pitch

    if ser and ser.in_waiting:
        try:
            line = ser.readline().decode('utf-8').strip()
            data = json.loads(line)
            angles = data.get("mpu", {}).get("angle", [0.0, 0.0])
            angle_roll = angles[0]
            angle_pitch = angles[1]
        except Exception as e:
            print(f"[WARN] Invalid line: {e}")

    glutPostRedisplay()
    glutTimerFunc(20, lambda x: update_data(), 0)

# ====== OpenGL Init ======
def init_opengl():
    glClearColor(0.1, 0.1, 0.1, 1.0)
    glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, 1.0, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)

# ====== Main Entry ======
def main():
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH)
    glutInitWindowSize(600, 600)
    glutCreateWindow(b"ROV Orientation Visualizer - Roll & Pitch")
    init_opengl()
    glutDisplayFunc(display)
    update_data()
    glutMainLoop()

if __name__ == "__main__":
    main()
