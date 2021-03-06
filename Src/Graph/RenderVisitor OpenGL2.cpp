#include "RenderVisitor.hpp"

void RenderVisitor::DrawOpenGL()
{
   DLOG(INFO) << "DrawOpenGL\n";

//    glMatrixMode(GL_PROJECTION);
// //   glLoadIdentity();
//    glLoadMatrixf( _projectionMatrix.GetMatrix() );


// //   gluPerspective(45.0f,(GLfloat)800/(GLfloat)600,0.1f,100.0f);

//    glMatrixMode(GL_MODELVIEW);
//    //  glLoadIdentity();
//    glLoadMatrixf( _modelViewMatrix.GetMatrix() );

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   

//    glTranslatef(-1.5f,0.0f,-6.0f);	
//    glBegin(GL_TRIANGLES);
//    glVertex3f( 0.0f, 1.0f, 0.0f);	
//    glVertex3f(-1.0f,-1.0f, 0.0f);	
//    glVertex3f( 1.0f,-1.0f, 0.0f);	
//    glEnd();
   
 M3DVector3f lightPos;   // array of 3 float (not GLfloat)
 M3DVector4f color;      // array of 4 =||=
 
 // set light position and diffuse light color
  _light.GetDiffuse( color );
  _light.GetPosition( lightPos );
  
  GLBatch triangleBatch;
  GLfloat vVerts[] = { -0.5f, 0.0f, 0.0f,
                        0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f };
  triangleBatch.Begin(GL_TRIANGLES, 3); 
  triangleBatch.CopyVertexData3f(vVerts); 
   triangleBatch.End();
  
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
      _shaderManager->UseStockShader(GLT_SHADER_FLAT,
                                   _transformPipeline.GetModelViewProjectionMatrix(),
                                   color);

   //  _shaderManager->UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
   //                                _transformPipeline.GetModelViewMatrix(),
   //                                _transformPipeline.GetProjectionMatrix(),
   //                                lightPos, color);
  
  // GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
   // _shaderManager->UseStockShader(GLT_SHADER_IDENTITY, vRed);

  triangleBatch.Draw();
}

void CopyM3DVector3f(const float* source, M3DVector3f dest) {
	memcpy(dest, source, sizeof(M3DVector3f));
}

void CopyM3DVector2f(const float* source, M3DVector3f dest) {
	memcpy(dest, source, sizeof(M3DVector2f));
}


RenderVisitor::RenderVisitor( GLShaderManager* gm )
{
   _shaderManager = gm;
   _modelViewMatrix.LoadIdentity();
   _projectionMatrix.LoadIdentity();
   _transformPipeline.SetMatrixStacks( _modelViewMatrix, _projectionMatrix );
}

void RenderVisitor::Traverse( CompositeNode* c )
{
   //DLOG(INFO) << "RenderVisitor about to traverse node\n";
  NodeIterator it  = c->GetNodeIterator();
  NodeIterator end = c->GetNodeIteratorEnd();

//   DLOG(INFO) << (c == 0) << endl;
// //  DLOG(INFO) << "Composite children count: " <<
  // c->GetChildrenCount() << endl;
  // DLOG(INFO) << "Composite name: " << c->GetName() << endl;
  
  while ( it != end ) {
     (*it)->Accept( this );
     ++it;
  }
}

void RenderVisitor::VisitTransform( Transform* t )
{
  _modelViewMatrix.PushMatrix(); // save current matrix
  _modelViewMatrix.MultMatrix( t->GetMatrix() );

  Traverse( t );

  _modelViewMatrix.PopMatrix(); // restore matrix
}

void RenderVisitor::VisitGeometry( Geometry* g ) 
{

     // DrawOpenGL();
     // return;
   
   // DLOG(INFO) << "RenderVisitor::VisitGeometry()\n";
   GLTriangleBatch triangles;
   GLBatch batch;

   const Mesh* mesh = g->GetMesh();
   
  TriangleIteratorConst it  = mesh->GetTriangleIteratorConst(); 
  TriangleIteratorConst end = mesh->GetTriangleIteratorEndConst();
  
  M3DVector4f lightPos = { 0.0f, 1.0f, 5.0f, 1.0f };
  M3DVector4f color ;   
  
  // set light position and diffuse light color
  _light.GetDiffuse( color );
  //_light.GetPosition( lightPos );

  // create a triangle mesh and give the initial size
  triangles.BeginMesh( mesh->GetVertexCount() );
  
  batch.Begin( GL_TRIANGLES, mesh->GetVertexCount() );

  //DLOG(INFO) << "Vertices: " << mesh->GetVertexCount() << endl;
  //DLOG(INFO) << "Normals: " << mesh->GetNormalCount() << endl;
  //DLOG(INFO) << "Triangles: " << mesh->GetTriangleCount() << endl;
  M3DVector3f vertices[3];
  M3DVector3f normals[3];
  M3DVector2f texCoords[3];
     
   // end = it; 
   // end++;
   // end++;
  //end++;

  int currentTriangle = 0;
  int maxTriangle = mesh->GetTriangleCount() -1;
  
   while ( it != end ) {
    
      if ( currentTriangle > maxTriangle ) {
         DLOG(INFO) << "Index is wrong\n";
      }
      
      CopyM3DVector3f(mesh->GetVertex( (*it).vert1 ), vertices[0]);
      CopyM3DVector3f(mesh->GetVertex( (*it).vert2 ), vertices[1]);
      CopyM3DVector3f(mesh->GetVertex( (*it).vert3 ), vertices[2]);
     
      CopyM3DVector3f(mesh->GetNormal( currentTriangle ), normals[0]);
      CopyM3DVector3f(mesh->GetNormal( currentTriangle ), normals[1]);
      CopyM3DVector3f(mesh->GetNormal( currentTriangle ), normals[2]);  
    
     // CopyM3DVector2f(mesh->GetTextureCoord( (*it).vert1 ), normals[0]);
     // CopyM3DVector2f(mesh->GetTextureCoord( (*it).vert2 ), normals[1]);
     // CopyM3DVector2f(mesh->GetTextureCoord( (*it).vert3 ), normals[2]);

      // CopyM3DVector2f(mesh->GetTextureCoord( (*it).vert1 ), texCoords[0]);
      // CopyM3DVector2f(mesh->GetTextureCoord( (*it).vert2 ), texCoords[1]);
      // CopyM3DVector2f(mesh->GetTextureCoord( (*it).vert3 ), texCoords[2]);
    
       triangles.AddTriangle( vertices, normals, texCoords );

      batch.Vertex3fv(vertices[0]);
      batch.Vertex3fv(vertices[1]);
      batch.Vertex3fv(vertices[2]);
    
      batch.Normal3fv(normals[0]);
      batch.Normal3fv(normals[1]);
      batch.Normal3fv(normals[2]);
    
      ++currentTriangle;
      ++it;
   }

   batch.End();
    triangles.End();
  
  // DLOG(INFO) << "color[0]: " << color[0] << " color[1]: " << color[1]
  //            << " color[2]: " << color[2] << " color[3]: " << color[3]
  //            << endl;
   static GLfloat color1[] = { 0.0f, 1.0f, 0.0f, 1.0f };
  //     _shaderManager->UseStockShader(GLT_SHADER_FLAT,
  //                                     _transformPipeline.GetModelViewProjectionMatrix(),
   //                                 color1);

  // GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
  // _shaderManager->UseStockShader(GLT_SHADER_IDENTITY, vRed);

   M3DVector4f lightEyePos;
   m3dTransformVector4( lightEyePos, lightPos,
                        _transformPipeline.GetModelViewMatrix() );

  // load shader program
    _shaderManager->UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                   _transformPipeline.GetModelViewMatrix(),
                                   _transformPipeline.GetProjectionMatrix(),
                                   lightEyePos, color1);
  
     batch.Draw();
    //  triangles.Draw();
  // DLOG(INFO) << "triangles has been drawn\n";
}

void RenderVisitor::VisitLight( Light* l )
{
  _light = *l; // maybe accumulate light in a vector?
  Traverse( l );
}

/* TODO: what is group?
void RenderVisitor::VisitGroup( Group* g ) 
{
  _modelViewMatrix.PushMatrix(); // save current matrix
  _modelViewMatrix.MultMatrix( g->GetMatrix() );

  Traverse( g );

  _modelViewMatrix.PopMatrix(); // restore matrix
}
*/

void RenderVisitor::VisitCamera( Camera* c)
{
  // save current matrices
  _modelViewMatrix.PushMatrix();
  _projectionMatrix.PushMatrix();

  // retrieve view and projection matrices
  // maybe shouldn't accumulate them but replace current
  // matrices? Does it make sense to add transformations
  // for cameras?
  M3DMatrix44f matrix;
  c->GetViewMatrix(matrix);
  _modelViewMatrix.LoadMatrix( matrix );
  c->GetProjectionMatrix(matrix);
  _projectionMatrix.LoadMatrix( matrix );

  Traverse( c );

  _modelViewMatrix.PopMatrix();
  _projectionMatrix.PopMatrix();
}
