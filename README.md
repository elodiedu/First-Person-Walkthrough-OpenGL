**# First-Person-Walkthrough-OpenGL**


This project implements a first-person walkthrough of a 3D scene using OpenGL and GLSL. WASD controls allow the player to move around the environment, while the arrow keys enable the player to rotate the view upward, downward, left, and right.

The 2D viewing perspective is produced through a 3D view matrix and projection matrix. The view (camera) matrix is continuously updated in response to user interaction. The camera's up direction is recomputed using the cross product of the viewing and right directions, maintaining an orthonormal camera coordinate system and allowing the camera to rotate continuously through a full 360-degree range without encountering singularities.

The projection matrix is defined using a viewing frustum, which specifies the horizontal and vertical field of view as well as the near and far clipping planes, thereby determining the visible region of the rendered 3D scene.

The width and height of the final rendered image is adjustable according to the window frame size.

**Rendered Cathedral from example model


<img width="490" height="493" alt="2" src="https://github.com/user-attachments/assets/c3ab0002-dcf0-4636-90dc-dcbc843beab7" />

<img width="497" height="496" alt="Screenshot 2026-09-07 203658" src="https://github.com/user-attachments/assets/855949b7-033e-4e71-bbf8-109f71f2c47b" />









