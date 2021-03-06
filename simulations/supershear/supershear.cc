#include <iostream>
#include <fstream>
#include <Eigen/Eigen>
//include the bie header files
#include "material.hh"
#include "precomputed_kernel.hh"
#include "infinite_boundary.hh"
//include the fem header files
#include "mesh_Generated_multi_faults.hpp"
#include "bcdof.hpp"
#include "bcdof_ptr.hpp"
#include "cal_ke.hpp"
#include "cal_ezz.hpp"
#include "cal_fe_global_plastic.hpp"
#include "cal_fe_global_const_ke.hpp"
#include "mapglobal.hpp"
#include "Slip_Weakening_reg.hpp"
#include "Slip_Weakening.hpp"
#include "cal_slip_sliprate.hpp"
#include "time_advance.hpp"
#include "BIE_correct.hpp"
//#include "igl/list_to_matrix.h"
#include "Getplastic.hpp"
#include "cal_Bmat.hpp"
#include "Slip_Weakening_lumpM_with_buffer.hpp"


//#include <omp.h>
using namespace Eigen;
using namespace std;

typedef Eigen::Matrix<int, -1, -1,RowMajor> MatrixXi_rm;

void read_matrix(std::string fileName, Eigen::MatrixXd &outputMat);

double time_fem=0;
double time_bie;
int main() {
    bool vn_output_ON = true;
    bool ezz_output_ON = true;
    int output_freq = 100;
    
    double time_run = 2e-4;
    double S = 0.31;
    double sigma_abs_const = 17.59e6;
    // Slip weakening friction parameters
    double Dc = 30e-6;
    double mu_d_const = 0.25;
    double mu_s_const = 0.65;

    time_fem = 0.0;
    // Domain Size
    double x_min = -300.0e-3;
    double x_max = 300.0e-3;
    double y_min = -20e-3;
    double y_max = 20e-3;
    int dim = 2.0;
    double dx = 5.0e-4;
    double dy = 5.0e-4;
    int nx_el = (x_max-x_min)/dx;
    int ny = (y_max-y_min)/dy;
    MatrixXd Node = MatrixXd::Zero((nx_el+1)*(ny+1),2);
    MatrixXi_rm Element(nx_el*ny,4); Element.setZero();
    ArrayXi BIE_top_surf_nodes = ArrayXi::Zero((nx_el+1),1);
    ArrayXi BIE_bot_surf_nodes = ArrayXi::Zero((nx_el+1),1);
    int num_faults = 1;
    // Mesh
    // Position of the fault : y_pos, x_left, x_right
    MatrixXd fault_pos(num_faults,3);
    fault_pos << 0.0,  x_min ,x_max;
    
    //    MatrixXd fault_pos;
    //    read_matrix("fault_pos.txt", fault_pos);
    //    int num_faults = fault_pos.rows();
    std::vector<std::vector<int>> fault_nodes(num_faults*2);
    mesh_Generated_multi_faults(x_min,x_max,y_min,y_max,dx,dy,nx_el,ny,Node, Element,BIE_top_surf_nodes, BIE_bot_surf_nodes,fault_pos, fault_nodes);
    
    std::ofstream Element_output("results/Element.txt");
    std::ofstream Node_output("results/Node.txt");
    Node_output<<Node;
    Element_output<<Element;
    
    int n_nodes = Node.rows();
    int n_el = Element.rows();
    int Ndofn = 2;
    int Nnel = Element.cols();
    // Material
    double density = 2670.0;
    double v_s =3.464e3;
    double v_p = 6.0e3;
    //double G= pow(v_s,2)*density;
    double G = 3.45e9;
    double nu = 0.25;
    double E =2.0*G*(1.0+nu);
    
    //double Lambda = pow(v_p,2)*density-2.0*G;
    //double E  = G*(3.0*Lambda+2.0*G)/(Lambda+G);
    //double nu = Lambda/(2.0*(Lambda+G));
    double h_star = G/((mu_s_const-mu_d_const)*sigma_abs_const/Dc);
    std::cout<<"h**="<< h_star<<"\n";
    std::cout<<"number of elements resolving the nulceation zone = " << int(h_star/(dx))<<std::endl;
    // Time
    double alpha = 0.4;
    double dt = alpha*dx/v_p;
    std::cout<<"dt="<<dt<<std::endl;
    //dt = 5e-4;
    double v_p_LVFZ = v_p;
    double v_s_LVFZ = v_s;
    double G_LVFZ= pow(v_s_LVFZ,2)*density;
    double Lambda_LVFZ = pow(v_p_LVFZ,2)*density-2.0*G_LVFZ;
  
    // Reyleigh Damping
    double beta =0.2;
    double q = beta*dt;
    int numt = time_run/dt;
   // numt =10;
    
    std::ofstream time_output("results/time.txt");
    time_output<<time_run<<"\n";
    time_output<<dt<<"\n";
    time_output<<numt<<std::endl;

    VectorXd time = dt*VectorXd::LinSpaced(numt,1,numt);

    // Intialization
    // disp velocity current and next time step (new)
    VectorXd u_n = VectorXd::Zero(n_nodes*Ndofn,1);
    VectorXd v_n = VectorXd::Zero(n_nodes*Ndofn,1);
    VectorXd u_new = VectorXd::Zero(n_nodes*Ndofn,1);
    VectorXd v_new = VectorXd::Zero(n_nodes*Ndofn,1);
    VectorXd a_n = VectorXd::Zero(n_nodes*Ndofn,1);
    // slip and slip-rate
    std::vector<Eigen::Array<double, -1, 1> > delt_u_n(num_faults);
    std::vector<Eigen::Array<double, -1, 1> > delt_v_n(num_faults);
    std::vector<Eigen::Array<double, -1, 1> > T_c(num_faults);
    std::vector<Eigen::Array<double, -1, 1> > T_0(num_faults);
    std::vector<Eigen::Array<double, -1, 1> > tau_s(num_faults);
    std::vector<Eigen::Array<double, -1, 1> > mu_s(num_faults);

    
    
    // Vector containing number of elements on each faults (nx)
    std::vector<int> nx_faults(num_faults);
    for (int i=0;i<num_faults;i++)
    {
        nx_faults[i] =fault_nodes[2*i].size();
    }
    
    
    for (int i=0;i<num_faults;i++)
    {
        delt_u_n[i] = ArrayXd::Zero(Ndofn*(nx_faults[i]),1);
        delt_v_n[i] = ArrayXd::Zero(Ndofn*(nx_faults[i]),1);
        T_c[i] = ArrayXd::Zero(Ndofn*(nx_faults[i]),1);
        T_0[i] = ArrayXd::Zero(Ndofn*(nx_faults[i]),1);
        tau_s[i] = mu_s_const*ArrayXd::Ones((nx_faults[i]),1)*sigma_abs_const;
        mu_s[i] = ArrayXd::Zero(nx_faults[i], 1);
    }
    
    // Making a copy of delt_u and delt_v to calculate the frictional disspation
    std::vector<Eigen::Array<double, -1, 1> > delt_u_n_copy(num_faults);
    std::vector<Eigen::Array<double, -1, 1> > delt_v_n_copy(num_faults);
    for (int i=0;i<num_faults;i++)
    {
        delt_u_n_copy[i] = ArrayXd::Zero(Ndofn*(nx_faults[i]),1);
        delt_v_n_copy[i] = ArrayXd::Zero(Ndofn*(nx_faults[i]),1);
    }
    // Write the x coordiantes for each fault
    std::vector<double> fault_angle(num_faults);
    double x1 = 0.0;
    double x2 = 0.0;
    double y1 = 0.0;
    double y2 = 0.0;
    for (int j=0;j<num_faults; j++)
    {
        VectorXd x = VectorXd::Zero(nx_faults[j], 1);
        VectorXd y = VectorXd::Zero(nx_faults[j], 1);
        
        for (int i=0; i<x.size(); i++)
        {
            x(i) = Node(fault_nodes[2*j][i],0);
            y(i) = Node(fault_nodes[2*j][i],1);
        }
        
        //std::ofstream x_output("results/fault_x_coord.txt");
        std::string  x_fault= "results/x_fault_"+std::to_string(j)+".txt";
        std::ofstream x_output(x_fault);
        x_output<<x;
        
        x1 = Node(fault_nodes[2*j][0],0);
        x2 = Node(fault_nodes[2*j][nx_faults[j]-1],0);
        y1 = Node(fault_nodes[2*j][0],1);
        y2 = Node(fault_nodes[2*j][nx_faults[j]-1],1);
        fault_angle[j]=std::atan2(y2-y1,x2-x1);
    }
    VectorXd F_ext_global = VectorXd::Zero(n_nodes*Ndofn,1);
    // Setting intial stress on the fault
    for (int j=0; j<num_faults;j++)
    {
        for (int i=0; i<nx_faults[j]; i++)
        {
            T_0[j](2*i+1) = -sigma_abs_const;
            
        }
        if (j==0)
        {
            mu_s[j] =mu_s_const*ArrayXd::Ones(nx_faults[j], 1);
        }
        else
        {
            mu_s[j] =mu_s_const*ArrayXd::Ones(nx_faults[j], 1);
        }
    }
    //Adding pertubation to the main fault 0
    VectorXd x_main = VectorXd::Zero(nx_faults[0], 1);
    
    
    for (int i=0; i<x_main.size(); i++)
    {
        x_main(i) = Node(fault_nodes[0][i],0);
    }
    
    
    double tau_0 =(mu_s_const*sigma_abs_const+ mu_d_const*sigma_abs_const*S)/(1.0+S);

    for (int k=0 ; k<nx_faults[0]; k++)
    {
        if ((x_main(k)<=(0.0+0.02))&&(x_main(k)>=(0.0-0.02)))
        {
            T_0[0](2*k) = sigma_abs_const*(mu_s_const)*(1.01);
        }
        else
        {
            T_0[0](2*k) = tau_0;
        }
    }
    // Get the index degree of freedome for each element
    VectorXi index_el = VectorXi::Zero(Ndofn*Nnel,1);
    MatrixXi index_store = MatrixXi::Zero(Nnel*Ndofn,n_el);
    for (int i=0;i<n_el;i++)
    {
        bcdof(Element.row(i),dim,index_el);
        index_store.col(i) = index_el;
    }
    VectorXi BIE_top_surf_index = VectorXi::Zero(Ndofn*(BIE_top_surf_nodes.size()),1);
    VectorXi BIE_bot_surf_index = VectorXi::Zero(Ndofn*(BIE_bot_surf_nodes.size()),1);
    
    std::vector<Eigen::ArrayXi> Fault_surf_index(num_faults*2);
    for (int i=0; i <num_faults*2; i++)
    {
        Fault_surf_index[i] = ArrayXi::Zero(Ndofn*(fault_nodes[i].size()),1);
    }
    
    // Getting the faults DOF index
    for (int i=0; i<num_faults*2;i++)
    {
        bcdof_ptr(fault_nodes[i], dim, Fault_surf_index[i].data());
    }
    bcdof(BIE_top_surf_nodes,dim,BIE_top_surf_index);
    bcdof(BIE_bot_surf_nodes,dim,BIE_bot_surf_index);
    // Calculating the Global Mass Vector (lumped mass)
    // Element mass
    double M=density*dx*dy*1.0;
    VectorXd M_el_vec = M/4*VectorXd::Ones(Nnel*Ndofn,1);
    VectorXd M_global_vec=VectorXd::Zero(n_nodes*Ndofn,1);
    for (int i=0 ; i<n_el;i++)
    {
        index_el = index_store.col(i);
        mapglobal(index_el,M_global_vec,M_el_vec);
    }
    // Element matrix
    MatrixXd ke = MatrixXd::Zero(8,8);
    MatrixXd coord = MatrixXd::Zero(4,2);
    VectorXi Element_0= Element.row(0);
    coord.row(0) = Node.row(Element_0(0));
    coord.row(1) = Node.row(Element_0(1));
    coord.row(2) = Node.row(Element_0(2));
    coord.row(3) = Node.row(Element_0(3));
    
    
    cal_ke (coord,E,nu,ke);
    
    double detJ = 0.0;
    std::vector<Eigen::MatrixXd> B_mat(4);
    cal_Bmat(coord, E, nu, B_mat, detJ);
    // BIE part initiation
    // Setting up the material property for the BIE code
    Material BIE_top_mat = Material(E,nu,density);
    Material BIE_bot_mat = Material(E,nu,density);
    double length = x_max-x_min;
    // infinte bc BIE call infinite_boundary.cc
    PrecomputedKernel h11("kernels/nu_.25_h11.dat");
    PrecomputedKernel h12("kernels/nu_.25_k12.dat");
    PrecomputedKernel h22("kernels/nu_.25_h22.dat");
    InfiniteBoundary BIE_inf_top(length,nx_el+1,1.0,&BIE_top_mat,&h11,&h12,&h22);
    InfiniteBoundary BIE_inf_bot(length,nx_el+1,-1.0,&BIE_bot_mat,&h11,&h12,&h22);
    // BIE setting time step
    BIE_inf_top.setTimeStep(dt);
    BIE_inf_bot.setTimeStep(dt);
    // BIE initialization
    BIE_inf_top.init();
    BIE_inf_bot.init();
    printf("ready to start\n");
    // Output
    ofstream file;
    file.open("results/num_nodes_fault.bin",ios::binary);
    file.write((char*)(nx_faults.data()),nx_faults.size()*sizeof(int));
    file.close();
    //
    file.open("results/u_n.bin",ios::binary);
    file.close();
    file.open("results/v_n.bin",ios::binary);
    file.close();
    file.open("results/eq_ep_n.bin",ios::binary);
    file.close();
    
    // output BIE boundary u_n v_n
    VectorXd BIE_top_u_n = VectorXd::Zero(2*(nx_el+1),1);
    VectorXd BIE_bot_u_n = VectorXd::Zero(2*(nx_el+1),1);
    VectorXd BIE_top_v_n = VectorXd::Zero(2*(nx_el+1),1);
    VectorXd BIE_bot_v_n = VectorXd::Zero(2*(nx_el+1),1);
    // Add output files BIE_top BIE_bot u_n v_n
    file.open("results/BIE_top_u_n.bin",ios::binary | ios::app);
    file.close();
    file.open("results/BIE_top_v_n.bin",ios::binary | ios::app);
    file.close();
    file.open("results/BIE_bot_u_n.bin",ios::binary | ios::app);
    file.close();
    file.open("results/BIE_bot_u_n.bin",ios::binary | ios::app );
    file.close();
    
    
    for (int i=0;i<num_faults;i++)
    {
        std::string slip = "results/slip_"+std::to_string(i)+".bin";
        file.open(slip);
        file.close();
        std::string slip_rate = "results/slip_rate_"+std::to_string(i)+".bin";
        file.open(slip_rate);
        file.close();
        std::string shear = "results/shear_"+std::to_string(i)+".bin";
        file.open(shear);
        file.close();
    }
    // double start = omp_get_wtime();
    std::vector<MatrixXd> strain_n_store(n_el);
    std::vector<MatrixXd> stress_n_store(n_el);
    
    for (int i=0; i<n_el; i++)
    {
        strain_n_store[i] = MatrixXd::Zero(3, 4);
        stress_n_store[i] = MatrixXd::Zero(3, 4);
        
    }
    
    // Equivalent plastic strain
    std::vector<double> eq_ep_out(n_el);
    std::vector<double> ezz_out(n_el);
    std::vector<double> exx_out(n_el);
    std::vector<double> eyy_out(n_el);


    file.open("results/ezz.bin",ios::binary);
    file.close();
    file.open("results/exx.bin",ios::binary);
    file.close();
    file.open("results/eyy.bin",ios::binary);
    file.close();

    // Equivalent plastic strain
    // Total Energy Dissipation
    double E_p_out;
    // Total Frictional Dissipation
    double E_fric_sec=0.0;
    double E_fric_main=0.0;
    double avg_slip = 0.0;
    double max_slip = 0.0;
    std::ofstream E_p_output("results/E_p.txt");
    std::ofstream E_fric_sec_output("results/E_fric_sec.txt");
    std::ofstream E_fric_main_output("results/E_fric_main.txt");
    std::ofstream avg_slip_main_output("results/avg_slip_main.txt");
    std::ofstream max_slip_rate_main_output("results/max_slip_rate_main.txt");
    // Main time loop
    for (int j=0;j<numt;j++)
    {
        // Compute the global internal force
        VectorXd fe_global= VectorXd::Zero(n_nodes*Ndofn,1);
          cal_fe_global_const_ke(n_nodes, n_el, index_store, q, u_n, v_n, Ndofn, ke, fe_global);
        //cal_fe_global_plastic(n_el, index_store, ke,B_mat, detJ, E, nu, q, u_n, v_n, Ndofn, fe_global, strain_n_store,stress_n_store,
          //                    sxx_initial, syy_initial, sxy_initial, cohes, blkfric,eq_ep_out);
        // Friction subroutine
        VectorXd F_total = F_ext_global-fe_global;
        for (int i=0 ; i<num_faults; i++)
        {
            VectorXd F_fault = VectorXd::Zero(Ndofn*(nx_faults[i]),1);
            VectorXd M_pos = VectorXd::Zero(Ndofn*nx_faults[i], 1);
            VectorXd M_neg = VectorXd::Zero(Ndofn*nx_faults[i],1);
            
            maplocal(Fault_surf_index[2*i], M_global_vec, M_pos);
            maplocal(Fault_surf_index[2*i+1], M_global_vec, M_neg);
            Slip_Weakening_lumpM_with_buffer(M_global_vec, Fault_surf_index[2*i], Fault_surf_index[2*i+1], fe_global, dt, dx, dy, nx_faults[i]-1, delt_v_n[i], delt_u_n[i], T_0[i], tau_s[i], mu_s[i], mu_d_const, Dc, Ndofn, M_pos, M_neg , F_fault, T_c[i], fault_angle[i]);
            mapglobal(Fault_surf_index[2*i],F_total,-F_fault);
            mapglobal(Fault_surf_index[2*i+1],F_total,F_fault);
        }
        
        // Central Difference Time integration
        time_advance(u_n, v_n, F_total, M_global_vec, dt);
        // Get the slip and slip rate
        
        for (int i=0; i<num_faults; i++)
        {
            cal_slip_slip_rate(u_n, v_n,  Fault_surf_index[2*i], Fault_surf_index[2*i+1] , Ndofn, nx_faults[i]-1, delt_u_n[i], delt_v_n[i]);
        }
        // Correct the BIE surf nodes solutions from FEM with the BIE solution
        BIE_correct(BIE_top_surf_index, BIE_bot_surf_index, fe_global, Ndofn, nx_el, dx, BIE_inf_top, BIE_inf_bot, u_n, v_n);
        
        // Calculating the frictional disspation for the seondary faults and main faults
        // Secondary faults disspaltions
        for (int i=1;i<num_faults;i++)
        {
            VectorXd E_fric_sec_temp = VectorXd::Zero(nx_faults[i],1);
            for (int k=0; k<nx_faults[i];k++)
            {
                E_fric_sec_temp(k) = (delt_u_n[i](2*k)-delt_u_n_copy[i](2*k))*T_c[i](2*k);
            }
            E_fric_sec +=E_fric_sec_temp.sum();
        }
        
        E_fric_sec_output<<E_fric_sec<<std::endl;
        
        for (int i=0;i<1;i++)
        {
            VectorXd E_fric_main_temp = VectorXd::Zero(nx_faults[i],1);
            for (int k=0; k<nx_faults[i];k++)
            {
                E_fric_main_temp(k) = (delt_u_n[i](2*k)-delt_u_n_copy[i](2*k))*T_c[i](2*k);            }
            E_fric_main +=E_fric_main_temp.sum();
            
            VectorXd delt_u_n_temp = VectorXd::Zero(nx_faults[i],1);
            VectorXd delt_v_n_temp = VectorXd::Zero(nx_faults[i],1);
            
            for (int k=0; k<nx_faults[i];k++)
            {
                delt_u_n_temp(k) = delt_u_n[i](2*k);
                delt_v_n_temp(k) = delt_v_n[i](2*k);
                
            }
            avg_slip = delt_u_n_temp.sum()/nx_faults[i];
            max_slip = delt_v_n_temp.maxCoeff();
            
        }
        E_fric_main_output<<E_fric_main<<std::endl;
        avg_slip_main_output<<avg_slip<<std::endl;
        max_slip_rate_main_output<<max_slip<<std::endl;
        
        
        for (int i=0;i<1;i++)
        {
            std::string slip = "results/slip_"+std::to_string(i)+".bin";
            file.open(slip,ios::binary | ios::app);
            file.write((char*)(delt_u_n[i].data()),delt_u_n[i].size()*sizeof(double));
            file.close();
            
            std::string slip_rate = "results/slip_rate_"+std::to_string(i)+".bin";
            file.open(slip_rate,ios::binary | ios::app);
            file.write((char*)(delt_v_n[i].data()),delt_v_n[i].size()*sizeof(double));
            file.close();
            
            std::string shear = "results/shear_"+std::to_string(i)+".bin";
            file.open(shear,ios::binary | ios::app);
            file.write((char*)(T_c[i].data()),T_c[i].size()*sizeof(double));
            file.close();
        }
//
        if (vn_output_ON){
            if (j%output_freq==1)
            {
    //            file.open("results/u_n.bin",ios::binary | ios::app);
    //            file.write((char*)(u_n.data()),u_n.size()*sizeof(double));
    //            file.close();
                file.open("results/v_n.bin",ios::binary | ios::app);
                file.write((char*)(v_n.data()),v_n.size()*sizeof(double));
                file.close();
            }
        }
//        //  Adding virtual boundary information
//        maplocal(BIE_top_surf_index,u_n,BIE_top_u_n);
//        maplocal(BIE_top_surf_index,v_n,BIE_top_v_n);
//        maplocal(BIE_bot_surf_index,u_n,BIE_bot_u_n);
//        maplocal(BIE_bot_surf_index,v_n,BIE_bot_v_n);
//
//
//        file.open("results/BIE_top_u_n.bin",ios::binary | ios::app);
//        file.write((char*)(BIE_top_u_n.data()),BIE_top_u_n.size()*sizeof(double));
//        file.close();
//        file.open("results/BIE_top_v_n.bin",ios::binary | ios::app);
//        file.write((char*)(BIE_top_v_n.data()),BIE_top_v_n.size()*sizeof(double));
//        file.close();
//        file.open("results/BIE_bot_u_n.bin",ios::binary | ios::app);
//        file.write((char*)(BIE_bot_u_n.data()),BIE_bot_u_n.size()*sizeof(double));
//        file.close();
//        file.open("results/BIE_bot_v_n.bin",ios::binary | ios::app);
//        file.write((char*)(BIE_bot_v_n.data()),BIE_bot_v_n.size()*sizeof(double));
//        file.close();
////
     //   cal_ezz(coord, E, nu, ezz_out, n_el, index_store, q, u_n);
        if (ezz_output_ON){
            if (j%output_freq==1)
            {
                cal_ezz(coord, E, nu, ezz_out, exx_out, eyy_out, n_el, index_store, q, u_n);
                file.open("results/ezz.bin",ios::binary | ios::app);
                file.write((char*)(ezz_out.data()),ezz_out.size()*sizeof(double));
                file.close();
                //            file.open("results/exx.bin",ios::binary | ios::app);
                //            file.write((char*)(exx_out.data()),exx_out.size()*sizeof(double));
                //            file.close();
                //            file.open("results/eyy.bin",ios::binary | ios::app);
                //            file.write((char*)(eyy_out.data()),eyy_out.size()*sizeof(double));
                //            file.close();
            }
        }
        printf("Simulation time = %e\n",time(j));
    }
    return 0;
}
