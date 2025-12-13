from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import serial
import re

# ====== Serial Config ======
SERIAL_PORT = "COM3"
BAUD_RATE = 115200

angle_yaw = 0.0

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"[ERROR] Serial not connected: {e}")
    ser = None

def draw_axes(length=2.0):
    glBegin(GL_LINES)
    glColor3f(1, 0, 0)
    glVertex3f(0, 0, 0)
    glVertex3f(length, 0, 0)
    glColor3f(0, 1, 0)
    glVertex3f(0, 0, 0)
    glVertex3f(0, length, 0)
    glColor3f(0, 0, 1)
    glVertex3f(0, 0, 0)
    glVertex3f(0, 0, length)
    glEnd()

def draw_cube():
    glColor4f(0.6, 0.7, 1.0, 1.0)
    glutSolidCube(1.0)

def display():
    global angle_yaw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -5.0)
    glRotatef(angle_yaw, 0.0, 0.0, 1.0)
    draw_axes()
    draw_cube()
    glutSwapBuffers()

def update_data():
    global angle_yaw

    if ser and ser.in_waiting:
        try:
            line = ser.readline().decode("utf-8").strip()
            match = re.search(r"Azimuth:\s*(-?\d+)", line)
            if match:
                angle_yaw = float(match.group(1))
        except:
            pass

    glutPostRedisplay()
    glutTimerFunc(5, lambda x: update_data(), 0)  # أقل delay = أسرع تحديث

def init_opengl():
    glClearColor(0.1, 0.1, 0.1, 1.0)
    glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, 1.0, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)

def main():
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH)
    glutInitWindowSize(600, 600)
    glutCreateWindow(b"Compass Yaw Visualizer - Fast")
    init_opengl() 
    glutDisplayFunc(display)
    update_data()
    glutMainLoop()

if __name__ == "__main__":
    main()
