#pragma once

#include "CompositeNode.hpp"
#include "../Include/GLFrame.h"
#include "../Include/GLFrustum.h"

/**
 * Camera class for easy handling of view space and projection.
 * The class wraps functionality for viewspace and projection
 * transformations. 
 * A frame handles the viewspace transformations and a frustum the
 * projection.
 * Every scene graph should have a default camera as it's root node
 * or at least above the geometry nodes to be rendered.
 */
class Camera : public CompositeNode {
public:

  /**
   * Create a default camera a the origin, pointing down -z axis
   * and having the y axis as the up vector.
   * Default view frustum gives a orthographic projection matrix.
   */
  Camera();

  /**
   * Create a camera and initilize it with a position and
   * a direction and set a node as a parent.
   * The y axis will be used as up vector.
   *
   * @param M3DVector3f the position of the camera.
   * @param M3DVector3f the direction of the camera.
   * @param Node        a pointer to the parent node.
   */
  Camera( const M3DVector3f, const M3DVector3f, Node* );

   ~Camera() { DLOG(INFO) << "DELETING CAMERA!\n!"; }

  /** Accept a visitor */
  virtual void Accept( NodeVisitor* );

  /** Set a new position */
  void SetPosition( const M3DVector3f );

  /** Set a new point to look at */
  void LookAt( const M3DVector3f );

  // void GetViewMatrix( const M3DMatrix44f m ) const;

  /**
   * Get the value of the view matrix.
   */
   void GetViewMatrix( M3DMatrix44f ) const;

   /** Get the value of the projection matrix */
  void GetProjectionMatrix( M3DMatrix44f );
  // const GLFrustum& GetViewFrustum() const;
  //void SetViewFrustum( GLFrustum& );

   /** 
   * Set the perspective projection in the frustum.
   *
   * @param GLfloat field of vision.
   * @param GLfloat aspect ratio.
   * @param GLfloat near clipping plane.
   * @param GLfloat far clipping plane.
   */
  void SetPerspective(GLfloat fFov, GLfloat fAspect, GLfloat fNear, GLfloat fFar);

  /**
   * Set the orthographic projection of the frustum.
   * 
   * @param GLfloat minimum x.
   * @param GLfloat maximum x.
   * @param GLfloat minimum y.
   * @param GLfloat maximum y.
   * @param GLfloat minimum z.
   * @param GLfloat maximum z.
   */
  void SetOrthographic(GLfloat xMin, GLfloat xMax, 
		       GLfloat yMin, GLfloat yMax,
		       GLfloat zMin, GLfloat zMax);

private:
  GLFrame      _frame;       /**< Handles movement and direction of camera */
  M3DMatrix44f _viewMatrix;  /**< Matrix is derived from the frame */
  GLFrustum    _viewFrustum; /**< Handles projection **/
};
