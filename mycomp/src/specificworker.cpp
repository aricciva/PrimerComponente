/*
 *    Copyright (C) 2015 by YOUR NAME HERE
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "specificworker.h"

#include <qt4/QtCore/qlist.h>

/**
* \brief Default constructor
*/
SpecificWorker::SpecificWorker(MapPrx& mprx) : GenericWorker(mprx)
{
  inner= new InnerModel("/home/salabeta/robocomp/files/innermodel/simpleworld.xml");
  

}

/**
* \brief Default destructor
*/
SpecificWorker::~SpecificWorker()
{

	  
}


void SpecificWorker::Transformaciones()
{

RTMat RoboCama,CamaApril, Inv_RoboCama, Inv_CamaApril;
InnerModelTransform* April =inner->newTransform ("April_id", "static", inner->getNode("rgbd"), 0, 0, 0, 0, 0, 0,0);


CamaApril=inner->getTransformationMatrix("April_id","rgbd");
RoboCama=inner->getTransformationMatrix("rgbd", "base");
 
 
 
  Inv_CamaApril=CamaApril.invert();
  InnerModelTransform *CamaraVirtual,*BaseVirtual;
  Inv_RoboCama=RoboCama.invert();

 QVec datos_M = Inv_CamaApril.getTr();
 QVec datos_R = Inv_RoboCama.getTr(); 
 
 
 switch(recorrido)
 {
   case 0:
     CamaraVirtual =inner->newTransform ("Virtual_id", "static", inner->getNode("target00"), datos_M.x(),datos_M.y() ,datos_M.z(),
					 Inv_CamaApril.getRxValue(),Inv_CamaApril.getRyValue(),Inv_CamaApril.getRzValue(), 0);
     break;
   case 1:
     CamaraVirtual =inner->newTransform ("Virtual_id", "static", inner->getNode("target01"), datos_M.x(),datos_M.y() ,datos_M.z(),
					 Inv_CamaApril.getRxValue(),Inv_CamaApril.getRyValue(),Inv_CamaApril.getRzValue(), 0);
     break;
   case 2:
      CamaraVirtual =inner->newTransform ("Virtual_id", "static", inner->getNode("target02"), datos_M.x(),datos_M.y() ,datos_M.z(),
					 Inv_CamaApril.getRxValue(),Inv_CamaApril.getRyValue(),Inv_CamaApril.getRzValue(), 0);
     break;
   case 3:
      CamaraVirtual =inner->newTransform ("Virtual_id", "static", inner->getNode("target03"), datos_M.x(),datos_M.y() ,datos_M.z(),
					 Inv_CamaApril.getRxValue(),Inv_CamaApril.getRyValue(),Inv_CamaApril.getRzValue(), 0);
     break;
}

BaseVirtual = inner->newTransform ("base_id", "static", inner->getNode("Virtual_id"), datos_M.x(),datos_M.y() ,datos_M.z(),
					 Inv_CamaApril.getRxValue(),Inv_CamaApril.getRyValue(),Inv_CamaApril.getRzValue(), 0);


InnerModelTransform* Robot =inner->newTransform ("Robot_id", "static", inner->getNode("rgbd"), datos_R.x(),datos_R.y() ,datos_R.z(),
					 Inv_RoboCama.getRxValue(),Inv_RoboCama.getRyValue(),Inv_RoboCama.getRzValue(), 0);




QVec valores = inner->transform("world",QVec::zeros(6),"Robot_id");
 inner->updateTransformValues("base",valores.x(),valores.y(),valores.z(),valores.rx(),valores.ry(),valores.rz());

 inner->removeNode("Robot_id");
 inner->removeNode("Virtual_id");
  
 inner->removeNode("April_id");

}


bool SpecificWorker::setParams(RoboCompCommonBehavior::ParameterList params)
{	
	timer.start(Period);
	return true;
}

void SpecificWorker::compute()
{
  try{     
	   ldata = laser_proxy->getLaserData();
	   TBaseState tbase; differentialrobot_proxy->getBaseState(tbase);
	  inner->updateTransformValues("base",tbase.x,0,tbase.z,0,tbase.alpha,0);
  
	   
	//Transformaciones();
	 
	 
     switch(state){

       case State::INIT:
	  std::cout << "<--------------Creando Camino---------->"<< std::endl;  
	  CrearCamino();
	      break;

        case State::SEARCH:
	 
	         search();
	      break;

	case State::ADVANCE:
	 
	  Controller();
		
		//controller_proxy->go();
	      break;
	      
        case State::STOP:
	         differentialrobot_proxy->setSpeedBase(0,0);
	         std::cout << "<--FIN-->"<< std::endl;   
	      break;
      }
  }catch(const Ice::Exception &ex){
     std::cout << ex << std::endl;
  }	 
}



void SpecificWorker::Controller()
{

  NavState st;
  st=controller_proxy->getState();

 if(!enviado){
 enviado=true;
 std::cout << "Enviando" << std::endl;
 TBaseState tbase; differentialrobot_proxy->getBaseState(tbase);
 DatosCamara::MyTag B= marcas.get(recorrido);

 
 QVec realidad = inner->transform("world",QVec::vec3(B.dist_x,0,B.dist_z),"rgbd"); 

 std::cout << "<--Posicion de la marca en APRIL-->"<< std::endl;   

 std::cout << "<--X:-->"<<B.dist_x<< std::endl;
  
 std::cout << "<--Z:-->"<<B.dist_z<< std::endl;
 
 
 std::cout << "<--Posicion de la marca en el mundo -->"<< std::endl;   

 std::cout << "<--X:-->"<<realidad.x()<< std::endl;
  
 std::cout << "<--Z:-->"<<realidad.z()<< std::endl;
 
 
 QVec ficcion = inner->transform("rgbd",QVec::vec3(B.dist_x,0,B.dist_z),"world");

  // QVec prueba1 = inner->transform("rgbd",QVec::vec3(tbase.x,0,tbase.z),"world"); /*Posicion de la camra segun el robot; Valores Estaticos X--> 0 Z --> 180*/
 
 
 
 std::cout << "<--Posicion del robot en el mundo-->"<< std::endl;   

 std::cout << "<--X:-->"<<tbase.x<< std::endl;
  
 std::cout << "<--Z:-->"<<tbase.z<< std::endl;
  
 
 std::cout << "<--¿Valores desconocidos? -->"<< std::endl;   

 std::cout << "<--X:-->"<<ficcion.x()<< std::endl;
  
 std::cout << "<--Z:-->"<<ficcion.z()<< std::endl;
 
 
 
 
 std::cout << "<////\\\\"<< std::endl;

 
 
  marcas.clear();
  st.state = "FIN";

   TargetPose TP;
   TP.x=realidad.x();
   TP.z=realidad.z();

  
   controller_proxy->go(TP);
}

  
  if(st.state == "FIN"){
    std::cout<<"FIN de la prueba"<<std::endl;
   state=State::SEARCH;
   enviado=false;
   recorrido++;
  }
  
  if(recorrido==4)
    state=State::STOP;
  
  
}

void SpecificWorker::search()
{
  

  
   differentialrobot_proxy->setSpeedBase(0, 0);
   DatosCamara::MyTag A;
 //  tag B;
    //  std::cout << "<--Buscando . . . -->"<< std::endl;   
   if(marcas.contains(recorrido)){
      //Obtenemos la marca
       A= marcas.get(recorrido);
      //Acutalizar Marca en memoria de inner
       Memoria.vec = inner->transform("world",QVec::vec3(A.dist_x,0,A.dist_z),"rgbd");
       Memoria.activo=true;
       std::cout << "<--Busqueda finalizada-->"<< std::endl;   
       state = State::ADVANCE; 
   }else{
	        differentialrobot_proxy->setSpeedBase(0,0.7707);
        }
    
}


void SpecificWorker::CrearCamino()
{

    Graph g;
  
  Node s=g.addNode();
  Node v2=g.addNode();
  Node v3=g.addNode();
  Node v4=g.addNode();
  Node v5=g.addNode();
  Node t=g.addNode();

  Edge s_v2=g.addEdge(s, v2);
 // Edge v2_s=g.addEdge(v2,s);
  Edge s_v3=g.addEdge(s, v3);
  Edge v2_v4=g.addEdge(v2, v4);
  Edge v2_v5=g.addEdge(v2, v5);
  Edge v3_v5=g.addEdge(v3, v5);
  Edge v4_t=g.addEdge(v4, t);
  Edge v5_t=g.addEdge(v5, t);
    
  LengthMap length(g);

  length[s_v2]=10;
  length[s_v3]=10;
  length[v2_v4]=5;
  length[v2_v5]=8;
  length[v3_v5]=5;
  length[v4_t]=8;
  length[v5_t]=8;

  std::cout << "Hello World!" << std::endl;
  std::cout <<  std::endl;
  std::cout << "This is library LEMON here! We have a graph!" << std::endl;
  std::cout <<  std::endl;

  std::cout << "Nodes:";
  for (NodeIt i(g); i!=INVALID; ++i)
    std::cout << " " << g.id(i);
  std::cout << std::endl;

  std::cout << "Edges:";
  for (EdgeIt i(g); i!=INVALID; ++i)
    std::cout << " (" << g.id(g.u(i)) << "," << g.id(g.v(i)) << ")";
  std::cout << std::endl;
  std::cout <<  std::endl;

  std::cout << "There is a map on the edges (length)!" << std::endl;
  std::cout <<  std::endl;
  for (EdgeIt i(g); i!=INVALID; ++i)
    std::cout << "length(" << g.id(g.u(i)) << ","
              << g.id(g.v(i)) << ")="<<length[i]<<std::endl;

  std::cout << std::endl;

  
std::cout << "Dijkstra algorithm demo..." << std::endl;

    Dijkstra<Graph, LengthMap> dijkstra_test(g,length);
    
    dijkstra_test.run(v3);
    
    std::cout << "The distance of node t from node s: "
              << dijkstra_test.dist(v4) << std::endl;

    std::cout << "The shortest path from s to t goes through the following "
              << "nodes (the first one is t, the last one is s): "
              << std::endl;

    for (Node v=v4;v != v3; v=dijkstra_test.predNode(v)) {
      std::cout << g.id(v) << "<-";
    }
    
    std::cout << g.id(v3) << std::endl;  


//  ListDigraph::NodeMap<int> map(g);
    
  
  
}


////////////////////////////////////////////////////////////77
//////  EN EL HILO THE ICE
////////////////////////////////////////////////////////////

void SpecificWorker::newAprilTag(const tagsList &tags)
{
   for(auto t : tags){
	   marcas.add(t);
    }
  
}   


  
