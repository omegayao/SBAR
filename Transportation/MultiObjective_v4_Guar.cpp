//The following code is based on C++ and CPLEX 
//For the paper: Integrating Environmental and Social Impacts into Optimal Design of Guayule and Guar Supply Chains


#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <time.h>
#include <math.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <ilcplex/ilocplex.h>
#include <string.h>
#include <cstdlib>
#include <cmath>

using namespace std;

int main(int argc, char **argv)
{

//--------------------read input data file ---------------------
char *strr=argv[1];
cout<<"The input file is "<<strr<<endl;
ifstream iData(strr);

assert(iData);

	int Ver;		// Version of Data

//Sets BTGEP/UC
	int F;			//Farms  
	int I;			//Irrigation districts
	int B; 			//Biorefineries 
	int P;			//Bio-products
	int C;			//Customers
	int OMEGA;		//Adoption rates scenarios
	
	int f; 			//farms counter
	int i;			//irrigation district counter
	int b; 			//biorefineries counter
	int p; 			//bio-product counter
	int c;			//customer counter
	int omega;		//scenario counter


	//reading inputs
	//iData >> Ver;
	iData >> F;	
	iData >> I;						
	iData >> B;			
	iData >> P;			
	iData >> C;			
	iData >> OMEGA;		

	float Prob[OMEGA + 1];			//Probability of adoption rate scenario

	for (omega = 1; omega <= OMEGA; omega++){
		iData >> Prob[omega];
	}
	
	float Theta[3 + 1];		// Weighted factor
	
	for (i = 1; i <= 3; i++){
		iData >> Theta[i];
	}

	
	float CH;		//Guayule’s harvesting and hauling cost
	float CL;		//Guayule’s loading and unloading cost
	float CT;		//Transportation cost of guayule
	float K;		//Number of planning biorefineries
	
	iData >> CH;		
	iData >> CL;	
	iData >> CT;
	iData >> K;
	
	int F_i[F + 1];		//farm at irrigation district i
	float Y_f[F + 1];	//Yield of Guayule
	float Aom_f[F + 1][OMEGA + 1];	//Area of supply of farm
	float Pom_f[F + 1][OMEGA + 1];	//Guayule productivity of farm
	float U_f[F + 1];	// Irrigation water use by farm

	for (f = 1; f <= F; f++){
		iData >> F_i[f];
		iData >> Y_f[f];
		iData >> U_f[f];
		for (omega = 1; omega <= OMEGA; omega++){		
			iData >> Aom_f[f][omega];
			Pom_f[f][omega] = Y_f[f] * Aom_f[f][omega];
		}
	}
	
		
	float Cfix_b[B + 1];	// Fixed cost for biorefinery b construction
	float Cvar_b[B + 1];	// Variable cost for biorefinery b construction
	float Max_b[B + 1];		//Minimum annual capacity of biorefinery
	float Min_b[B + 1];		//Maximum annual capacity of biorefinery

	for (b = 1; b <= B; b++){
		iData >> Cfix_b[b];
		iData >> Cvar_b[b];	
		iData >> Max_b[b];
		iData >> Min_b[b];		
	}
	
	
	float W_i[I + 1];		//Water availability in irrigation district i
	
	for (i = 1; i <= I; i++){
		iData >> W_i[i];		
	}
	
	float CP;				//Biomass processing cost
	iData >> CP;		
	
	
	
	float Theta_p[P + 1];	// Conversion factor for bio-product p for biorefinery b
	
	for (p = 1; p <= P; p++){
		iData >> Theta_p[p];				
	}
	
	float D_pc[P + 1][C + 1];	//Demand of bio-product p for customer c

	for (c = 1; c <= C; c++){
		for (p = 1; p <= P; p++){
			iData >> D_pc[p][c];		
		}
	}
	
	float CD_p[P + 1];			//distribution cost for bio-product p
	
	for (p = 1; p <= P; p++){
		iData >> CD_p[p];		
	}
	
	float L_fb[F + 1][B + 1];	// Distance from farm f to biorefinery b
	
	for (f = 1; f <= F; f++){
		for (b = 1; b <= B; b++){
			iData >> L_fb[f][b];		
		}
	}
	
	float L_bc[B + 1][C + 1];	// Distance from biorefinery b to curtomer c
	
	for (b = 1; b <= B; b++){
		for (c = 1; c <= C; c++){
			//iData >> L_bc[b][c];		
		}
	}
	
	float Epsilon;		        //Emission price 
	float UB_Env;				//Upper-bound for emissions cost
	float Beta; 				//Transportation emission factor
	
	iData >> Epsilon;
	iData >> UB_Env;
	iData >> Beta;
	
	float Mu_b[B + 1];			//Construction emission factor for biorefinery b
	
	for (b = 1; b <= B; b++){
		iData >> Mu_b[b];		
	}
	
	float Gamma_p[P + 1];		//Distribution emission factor for bio-product p
	float Delta_p[P + 1];		//Production emission factor for bio-product p
	
	for (p = 1; p <= P; p++){
		iData >> Gamma_p[p];
		iData >> Delta_p[p];		
	}
	
	float EB_b[B + 1];				// Earning per work hour for biorefinery construction,
	float EH_f[F + 1];				// Earning per work hour for harvesting 
	float ET_f[F + 1];				// Earning per work hour for transportation 
	float HT;				// Number of work hours required for transportation
	float HH;				// Number of work hours required for harvesting
	float HB;				// Number of work hours required for biorefinery construction
	float LB_Soc;			// Lower-bound for social benefit
	
	for (b = 1; b <= B; b++){
		iData >> EB_b[b];		
	}
	
	for (f = 1; f <= F; f++){
		iData >> EH_f[f];		
	}
	
	for (f = 1; f <= F; f++){
		iData >> ET_f[f];		
	}
	
	iData >> HT;
	iData >> HH;
	iData >> HB;
	iData >> LB_Soc;
	
	float EP_b[B + 1];		// Earning per work hour for production of bio-product p
	float ED_b[B + 1];		// Earning per work hour for distribution of bio-product p	
	float HD_b[B + 1];		// Number of work hours required for distribution of bio-product p
	float HP_b[B + 1]; 		// Number of work hours required for production of bio-product p	

	for (b = 1; b <= B; b++){
		iData >> EP_b[b];
		iData >> ED_b[b];	
		iData >> HD_b[b];	
		iData >> HP_b[b];			
	}


	
iData.close();

//--------------------end of input data file--------------------
//--------------------------------------------------------------


//------------------CHECK INPUT DATA ---------------------------



cout << "---------------------------------------------- POWER SYSTEM ------------------------------------------------" << endl;
cout << "Irrigation districts: " << I << endl;
cout << "Biorefineries : " << B << endl;
cout << "Bio-products: " << P << endl;
cout << "Customers: " << C << endl;
cout << "Adoption rates scenarios: " << OMEGA << endl;
cout << "Farms: " << F << endl;

cout << "Weighted factor Theta: " << endl;
for (i = 1; i <= 3; i++){
	cout << Theta[i] << " "  << endl;
}

cout << "Guayule’s harvesting and hauling cost: " << CH << endl;
cout << "Guayule’s loading and unloading cost: " << CL << endl;
cout << "Transportation cost of guayule: " << CT << endl;
cout << "Number of planning biorefineries: " << K << endl;

cout << "For each farm: F_i, Y_f, Aom_f, Pom_f, U_f" << endl;
for (f = 1; f <= F; f++){
	cout << F_i[f] << " ";
	for (omega = 1; omega <= OMEGA; omega++){
	cout << Aom_f[f][omega] << " ";
	}
	cout << U_f[f] << endl;
}

// cout << "For each farm: Pom_f" << endl;
// for (f = 1; f <= F; f++){
	// for (omega = 1; omega <= OMEGA; omega++){
	// cout << Pom_f[f][omega] << " ";
	// }
	// cout << endl;
// }

// cout << "For each farm: Aom_f, Pom_f" << endl;
// for (f = 1; f <= F; f++){
	// for (omega = 1; omega <= OMEGA; omega++){
	// cout << Aom_f[f][omega] << " " << Pom_f[f][omega] << endl;
	// }
// }

cout << "For each biorefinery: Cfix_b, Cvar_b, Max_b, Min_b" << endl;
for (b = 1; b <= B; b++){
	cout << Cfix_b[b] << " " << Cvar_b[b] << " " << Max_b[b] << " " << Min_b[b] << endl;
}

cout << "Water availability: " << endl;
for (i = 1; i <= I; i++){
	cout << W_i[i] << " "  << endl;
}

cout << "Biomass processing cost: " << endl;
cout << CP << " "  << endl;


cout << "Conversion factor for bio-product p: " << endl;
for (p = 1; p <= P; p++){
	cout << Theta_p[p] << " ";
}cout << endl;


cout << "Demand of bio-product p for customer c: " << endl;
for (p = 1; p <= P; p++){
	for (c = 1; c <= C; c++){
		cout << D_pc[p][c] << " ";
	}cout << endl;
}

cout << "Distribution cost for bio-product p: " << endl;
for (p = 1; p <= P; p++){
	cout << CD_p[p] << " "  << endl;
}

cout << "Distance from farm f to biorefinery b: " << endl;
for (f = 1; f <= F; f++){
	for (b = 1; b <= B; b++){
		cout << L_fb[f][b] << " ";
	}cout << endl;
}

// cout << "Distance from biorefinery b to curtomer c: " << endl;
// for (b = 1; b <= B; b++){
	// for (c = 1; c <= C; c++){
		// cout << L_bc[b][c] << " ";
	// }cout << endl;
// }

cout << "Emission price: " << Epsilon << endl;
cout << "Upper-bound for emissions cost: " << UB_Env << endl;
cout << "Transportation emission factor: " << Beta << endl;

cout << "Construction emission factor for biorefinery b: " << endl;
for (b = 1; b <= B; b++){
	cout << Mu_b[b] << " "  << endl;
}

cout << "For every bio-product: Gamma_p, Delta_p " << endl;
for (p = 1; p <= P; p++){
	cout << Gamma_p[p] << " "  << Delta_p[p] << endl;
}

cout << "Earning per work hour for biorefinery construction, EB_b: " << endl;
for (b = 1; b <= B; b++){
	cout << EB_b[b] << endl;
}

cout << "Earning per work hour for harvesting, EH_f: " << endl;
for (f = 1; f <= F; f++){
	//cout << EH_f[f] << endl;
}

cout << "Earning per work hour for transportation, ET_f: " << endl;
for (f = 1; f <= F; f++){
	//cout << ET_f[f] << endl;
}

cout << "Number of work hours required for transportation: " << HT << endl;
cout << "Number of work hours required for harvesting: " << HH << endl;
cout << "Number of work hours required for biorefinery construction: " << HB << endl;
cout << "Lower-bound for social benefit: " << LB_Soc << endl;


cout << "For every bio-product: EP_b, ED_b, HD_b, HP_b" << endl;
for (b = 1; b <= B; b++){
	cout << EP_b[b] << " "  << ED_b[b] << " "  << HD_b[b] <<" "  << HP_b[b] << endl;
}


 
//-------------------END OF CHECK INPUT DATA--------------------
//output file
char outputfilename[40];
sprintf(outputfilename,"MultiObj_NM_ver%d.csv",Ver);
ofstream oFile(outputfilename);

//create the CPLEX variable
typedef IloArray<IloNumVarArray> IloNumVarArray2;
typedef IloArray<IloNumVarArray2> IloNumVarArray3;
typedef IloArray<IloNumVarArray3> IloNumVarArray4;

typedef IloArray<IloBoolVarArray> IloBoolVarArray2;

//Create the CPLEX environment 	
IloEnv env; 

	//ST Model creation
	IloModel SBARmodel(env);
   
	//Variables declaration
	
	//Planning Variables
	IloBoolVarArray x(env, B + 1);							//x[b]: binary decision 	
	IloNumVarArray cap(env, B + 1,0,IloInfinity,ILOFLOAT);	//cap[b]: Capacity of biorefinery

	
	//Transportation and distribution
	IloNumVarArray3 u(env, OMEGA + 1);					//u[omega][f][b]: Commodity flow of guayule from farm f to biorefinery b
	for(omega = 1 ; omega <= OMEGA; omega++){
		u[omega] = IloNumVarArray2(env, F + 1);
		for(f = 1 ; f <= F; f++){
			u[omega][f] = IloNumVarArray(env, B + 1,0,IloInfinity,ILOFLOAT);
		}
	}

	IloNumVarArray4 v(env, OMEGA + 1);					//v[omega][p][b][c]:  Commodity flow of bio-product p from biorefinery b to customer c
	for(omega = 1 ; omega <= OMEGA; omega++){
		v[omega] = IloNumVarArray3(env, P + 1);
		for(p = 1 ; p <= P; p++){
			v[omega][p] = IloNumVarArray2(env, B + 1);
			for(b = 1 ; b <= B; b++){
				v[omega][p][b] = IloNumVarArray(env, C + 1,0,IloInfinity,ILOFLOAT);
			}
		}
	}



cout << "Variables created" << endl;	
//***********************************************************************************************************************	
	//Objective function
		IloExpr OBJ(env);
	
		//Economic objective
		for (b = 1; b <= B; b++){
			OBJ += Theta[1] * ( (Cfix_b[b] * x[b]) + (Cvar_b[b] * cap[b]) ); 
		}
		
			//Harvesting Cost
			for (omega = 1; omega <= OMEGA; omega++){
				for(f = 1 ; f <= F; f++){
					for(b = 1 ; b <= B; b++){
						OBJ +=  Theta[1]*(      Prob[omega] * (CH + CL) * u[omega][f][b]   ); 
					}
				}			
			}
			
			//Transportation Cost
			for (omega = 1; omega <= OMEGA; omega++){
				for(f = 1 ; f <= F; f++){
					for(b = 1 ; b <= B; b++){
						OBJ += Theta[1]*(    Prob[omega] * ( (CT * L_fb[f][b]) + CP ) * u[omega][f][b]    ); 
					}
				}			
			}
			
			//Production and distribution Cost
			for (omega = 1; omega <= OMEGA; omega++){
				for(b = 1 ; b <= B; b++){
					for(p = 1 ; p <= P; p++){
						for(c = 1 ; c <= C; c++){
							OBJ += Theta[1]*(    Prob[omega] * CD_p[p]  * v[omega][p][b][c]    ); 
						}
					}			
				}
			}
	
		//Environmental objective
		for (b = 1; b <= B; b++){
			OBJ += Theta[2]*(    Epsilon * (Cfix_b[b] * x[b]) + (Cvar_b[b] * cap[b])     ); 
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(f = 1 ; f <= F; f++){
				for(b = 1 ; b <= B; b++){
					OBJ += Theta[2]*(   Epsilon *  Prob[omega] * (L_fb[f][b] * u[omega][f][b] * Beta)    ); 						
				}			
			}
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(b = 1 ; b <= B; b++){
				for(p = 1 ; p <= P; p++){
					for(c = 1 ; c <= C; c++){
						OBJ += Theta[2]*(   Epsilon * Prob[omega] * ( (L_bc[b][c] * Gamma_p[p]) + Delta_p[p]) * v[omega][p][b][c]      ); 
					}
				}			
			}
		}
	
		//Social objective
		for(b = 1 ; b <= B; b++){
			OBJ += Theta[3]*(   (EB_b[b] + HB) * x[b]    );
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(f = 1 ; f <= F; f++){
				for(b = 1 ; b <= B; b++){
					OBJ += Theta[3] * (    Prob[omega] * ( (EH_f[f] * HH) + (ET_f[f] * HT * L_fb[f][b] ) ) * u[omega][f][b]     ); 						
				}			
			}
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(b = 1 ; b <= B; b++){
				for(p = 1 ; p <= P; p++){
					for(c = 1 ; c <= C; c++){
						OBJ += Theta[3]*(    Prob[omega] * ( (EP_b[b] * HD_b[b]) + (ED_b[b] * HP_b[b] * L_bc[b][c]) )* v[omega][p][b][c]      ); 
					}
				}			
			}
		}
	
		SBARmodel.add(IloMinimize(env, OBJ));

		cout << "Objective Created" << endl;	
		
//***********************************************************************************************************************	
	//Constraints


	// Constraint (5a)
	IloExpr Const_5a(env);
	for(b = 1 ; b <= B; b++){
		Const_5a += x[b];
	}
	SBARmodel.add(Const_5a <= K);												
	Const_5a.end();

	
	// Constraint (5b-1)
	for(b = 1 ; b <= B; b++){
		SBARmodel.add( (Min_b[b] * x[b]) - cap[b] <= 0);
	}
	
	// Constraint (5b-2)
	for(b = 1 ; b <= B; b++){
		SBARmodel.add( cap[b] - (Max_b[b] * x[b]) <= 0);
	}	
	 
	// Constraint (6a)
	for (omega = 1; omega <= OMEGA; omega++){
		for(f = 1 ; f <= F; f++){
			IloExpr Const_6a(env);
			for(b = 1 ; b <= B; b++){
				Const_6a += u[omega][f][b];
			}
			SBARmodel.add( Const_6a <= Pom_f[f][omega]);
			Const_6a.end();
		}
	}
	
	// Constraint (6b)
	for (omega = 1; omega <= OMEGA; omega++){
		for (i = 1; i <= I; i++){
			IloExpr Const_6b(env);
			for(f = 1 ; f <= F; f++){
				if(F_i[f] == i){
					for(b = 1 ; b <= B; b++){
						Const_6b += u[omega][f][b] * U_f[f];
					}
				}
			}
			SBARmodel.add( Const_6b <= W_i[i]);
			Const_6b.end();
		}
	}
	
	// Constraint (7a)
	for (omega = 1; omega <= OMEGA; omega++){
		for(b = 1 ; b <= B; b++){
			IloExpr Const_7a(env);
			for(f = 1 ; f <= F; f++){
				Const_7a += u[omega][f][b];
			}
			Const_7a += - cap[b];
			SBARmodel.add( Const_7a == 0);
			Const_7a.end();
		} 
	}		

	// Constraint (7b)
	for (omega = 1; omega <= OMEGA; omega++){
		for(b = 1 ; b <= B; b++){
			for(p = 1 ; p <= P; p++){
				IloExpr Const_7b(env);	
				Const_7b += cap[b] * Theta_p[p];			
					for(c = 1 ; c <= C; c++){
						Const_7b += - v[omega][p][b][c]; 
					}					
				SBARmodel.add( Const_7b == 0);
				Const_7b.end();
			}
		} 
	}
	
	// Constraint (7c)
	for (omega = 1; omega <= OMEGA; omega++){
		for(f = 1 ; f <= F; f++){
			for(b = 1 ; b <= B; b++){
				SBARmodel.add( u[omega][f][b] - ( Max_b[b] * x[b]) <= 0);
			}
		}
	}
	
	// Constraint (7d)
	for (omega = 1; omega <= OMEGA; omega++){
		for(b = 1 ; b <= B; b++){
			IloExpr Const_7d(env);
			for(p = 1 ; p <= P; p++){
				for(c = 1 ; c <= C; c++){
					Const_7d += v[omega][p][b][c];
				}
			}
			SBARmodel.add( Const_7d - ( Max_b[b] * x[b]) <= 0);
			Const_7d.end();
		}		
	}
	
	
	// Constraint (7e)
	for (omega = 1; omega <= OMEGA; omega++){
		for(p = 1 ; p <= P; p++){
			for(c = 1 ; c <= C; c++){	
				IloExpr Const_7e(env);
				for(b = 1 ; b <= B; b++){
					Const_7e += v[omega][p][b][c];
				}
				SBARmodel.add( Const_7e >= D_pc[p][c]);
				Const_7e.end();
			}
		}
	} 
	
	// Constraint (8)
	IloExpr Const_8(env);
	for (b = 1; b <= B; b++){
		Const_8 += Epsilon * (Cfix_b[b] * x[b]) + (Cvar_b[b] * cap[b]); 
	}
	for (omega = 1; omega <= OMEGA; omega++){
		for(f = 1 ; f <= F; f++){
			for(b = 1 ; b <= B; b++){
				Const_8 += Prob[omega] * (L_fb[f][b] * u[omega][f][b] * Beta) ; 						
			}			
		}
	}
	for (omega = 1; omega <= OMEGA; omega++){
		for(b = 1 ; b <= B; b++){
			for(p = 1 ; p <= P; p++){
				for(c = 1 ; c <= C; c++){
					Const_8 += Prob[omega] * ( (L_bc[b][c] * Gamma_p[p]) + Delta_p[p]) * v[omega][p][b][c]; 
				}
			}			
		}
	}
	SBARmodel.add( Const_8 <= UB_Env);
	Const_8.end();
	
	// Constraint (9)
	IloExpr Const_9(env);
	for(b = 1 ; b <= B; b++){
		Const_9 += (EB_b[b] + HB) * x[b];
	}
	for (omega = 1; omega <= OMEGA; omega++){
		for(f = 1 ; f <= F; f++){
			for(b = 1 ; b <= B; b++){
				Const_9 += Prob[omega] * ((EH_f[f] * HH) + (ET_f[f] + HT) ) * L_fb[f][b] * u[omega][f][b]; 						
			}			
		}
	}
	for (omega = 1; omega <= OMEGA; omega++){
		for(b = 1 ; b <= B; b++){
			for(p = 1 ; p <= P; p++){
				for(c = 1 ; c <= C; c++){
					Const_9 += Prob[omega] * ( (EP_b[b] * HD_b[b]) + (ED_b[b] * HP_b[b] * L_bc[b][c]) )* v[omega][p][b][c]; 
				}
			}			
		}
	}
	SBARmodel.add( Const_9 >= LB_Soc);
	Const_9.end();
	
//*****************************************************************************************	
//-------------------------------------- CPLEX Models -------------------------------------
//*****************************************************************************************	

float Time;					//Parameters for recording time for Second-Third level
Time = 0;
float starttime; 


IloCplex cplexSBAR(SBARmodel);
IloBool successFS = false;
IloAlgorithm::Status status_FS;

//Export FS Model
cplexSBAR.exportModel("MultiObj_Guar_NM_ver.lp");

//set gap parameters
cplexSBAR.setParam(IloCplex::EpGap,0.01);//gap 
cplexSBAR.setParam(IloCplex::TiLim,21600);//limits time in seconds for FS_MP and returns best solution so far
//cplexSBAR.setParam(IloCplex::Param::Benders::Strategy, 3);
//cplexSBAR.setParam(IloCplex::PreInd,0);  

try{
	starttime = cplexSBAR.getTime();
	successFS = cplexSBAR.solve();
	Time = Time + cplexSBAR.getTime() - starttime;
	
cout << "The objective is: " << cplexSBAR.getValue(OBJ) << endl;
cout << "CPU time is: " << Time << endl;

}catch(IloException& e) {
	cout<<e.getMessage()<<endl;
}



//**************************************
//CALCULATE OBJECTIVE BREAKDOWN

float EconomicObjective, InstallationCost, HarvestingCost, TransportationCost, Production_DistributionCost;
float EnvironmentalObjective, SocialObjective, TotalObjective;

	//Installation Cost
	for (b = 1; b <= B; b++){
		InstallationCost += (Cfix_b[b] * cplexSBAR.getValue(x[b])) + (Cvar_b[b] * cplexSBAR.getValue(cap[b])); 
	}

oFile << "Installation Cost:," << InstallationCost << endl;
cout << "Installation Cost: " << InstallationCost << endl;
		
	//Harvesting Cost
	for (omega = 1; omega <= OMEGA; omega++){
		for(f = 1 ; f <= F; f++){
			for(b = 1 ; b <= B; b++){
				HarvestingCost += Prob[omega] * (CH + CL) * cplexSBAR.getValue(u[omega][f][b]); 
			}
		}			
	}

oFile << "Harvesting Cost:," << HarvestingCost << endl;
cout << "Harvesting Cost: " << HarvestingCost << endl;
	
	//Transportation Cost
	for (omega = 1; omega <= OMEGA; omega++){
		for(f = 1 ; f <= F; f++){
			for(b = 1 ; b <= B; b++){
				TransportationCost += Prob[omega] * ( (CT * L_fb[f][b]) + CP) * cplexSBAR.getValue(u[omega][f][b]); 
			}
		}			
	}
			
oFile << "Transportation and Processing Cost:," << TransportationCost << endl;
cout << "Transportation and Processing Cost: " << TransportationCost << endl;

	//Production and distribution Cost
	for (omega = 1; omega <= OMEGA; omega++){
		for(b = 1 ; b <= B; b++){
			for(p = 1 ; p <= P; p++){
				for(c = 1 ; c <= C; c++){
					Production_DistributionCost += Prob[omega] * CD_p[p]  * cplexSBAR.getValue(v[omega][p][b][c]); 
				}
			}			
		}
	}

oFile << "Distribution Cost:," << Production_DistributionCost << endl;
cout << "Distribution Cost: " << Production_DistributionCost << endl;

	EconomicObjective = InstallationCost + HarvestingCost + TransportationCost + Production_DistributionCost;

oFile << "Economic Objective:," << EconomicObjective << endl;
cout << "Economic Objective: " << EconomicObjective << endl;	

oFile << "Economic Objective x Theta[1]:," << EconomicObjective * Theta[1] << endl;
cout << "Economic Objective x Theta[1]: " << EconomicObjective * Theta[1] << endl;	


	
		//Environmental objective
		for (b = 1; b <= B; b++){
			EnvironmentalObjective += Epsilon * (Cfix_b[b] * cplexSBAR.getValue(x[b])) + (Cvar_b[b] * cplexSBAR.getValue(cap[b])); 
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(f = 1 ; f <= F; f++){
				for(b = 1 ; b <= B; b++){
					EnvironmentalObjective += Epsilon * Prob[omega] * (L_fb[f][b] * cplexSBAR.getValue(u[omega][f][b]) * Beta) ; 						
				}			
			}
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(b = 1 ; b <= B; b++){
				for(p = 1 ; p <= P; p++){
					for(c = 1 ; c <= C; c++){
						EnvironmentalObjective += Epsilon * Prob[omega] * ( (L_bc[b][c] * Gamma_p[p]) + Delta_p[p]) * cplexSBAR.getValue(v[omega][p][b][c]); 
					}
				}			
			}
		}
	
oFile << "Environmental Objective:," << EnvironmentalObjective << endl;
cout << "Environmental Objective: " << EnvironmentalObjective << endl;	

oFile << "Environmental Objective x Theta[2]:," << EnvironmentalObjective * Theta[2] << endl;
cout << "Environmental Objective x Theta[2]: " << EnvironmentalObjective * Theta[2] << endl;		
		
		//Social objective
		for(b = 1 ; b <= B; b++){
			SocialObjective += (EB_b[b] + HB) * cplexSBAR.getValue(x[b]);
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(f = 1 ; f <= F; f++){
				for(b = 1 ; b <= B; b++){
					SocialObjective += Prob[omega] * ((EH_f[f] * HH) + (ET_f[f] * HT * L_fb[f][b] ) ) * cplexSBAR.getValue(u[omega][f][b]); 						
				}			
			}
		}
		for (omega = 1; omega <= OMEGA; omega++){
			for(b = 1 ; b <= B; b++){
				for(p = 1 ; p <= P; p++){
					for(c = 1 ; c <= C; c++){
						SocialObjective += Prob[omega] * ( (EP_b[b] * HD_b[b]) + (ED_b[b] * HP_b[b] * L_bc[b][c]) )* cplexSBAR.getValue(v[omega][p][b][c]); 
					}
				}			
			}
		}

oFile << "Social Objective:," << SocialObjective << endl;
cout << "Social Objective: " << SocialObjective << endl;	

oFile << "Social Objective x Theta[3]:," << SocialObjective * Theta[3] << endl;
cout << "Social Objective x Theta[3]: " << SocialObjective * Theta[3] << endl;	

TotalObjective = EconomicObjective + EnvironmentalObjective + SocialObjective;

float TotalObjectiveTheta;

TotalObjectiveTheta = (EconomicObjective * Theta[1] ) + (EnvironmentalObjective * Theta[2] ) + ( SocialObjective * Theta[3] );

oFile << "Total Calculated Objective is:," << TotalObjective << endl;
cout << "Total Calculated Objective is: " << TotalObjective << endl;

oFile << "Total Calculated Objective x Theta is:," << TotalObjectiveTheta << endl;
cout << "Total Calculated Objective x Theta is: " << TotalObjectiveTheta << endl;

oFile << "The CPLEX Calculated objective is:," << cplexSBAR.getValue(OBJ) << endl;
oFile << "CPU time is:," << Time << endl;


oFile << "x[b]:" << endl;
for(b = 1 ; b <= B; b++){
	oFile << cplexSBAR.getValue(x[b]) << ",";
}oFile << endl;

oFile << "cap[b]:" << endl;
for(b = 1 ; b <= B; b++){
	oFile << cplexSBAR.getValue(cap[b]) << ",";
}oFile << endl;


oFile << "u[omega][f][b]:" << endl;
for (omega = 1; omega <= OMEGA; omega++){
	oFile << "omega:," << omega << endl;
	for(f = 1 ; f <= F; f++){
		oFile << "f:," << f << endl;
		for(b = 1 ; b <= B; b++){
			oFile << cplexSBAR.getValue(u[omega][f][b]) << ",";	
		}oFile << endl;
	}
	oFile << endl;
	oFile << endl;
}

oFile << "v[omega][p][b][c]:" << endl;
for (omega = 1; omega <= OMEGA; omega++){
	oFile << "omega:," << omega << endl;
	for(p = 1 ; p <= P; p++){
		oFile << "p:," << p << endl;
		for(b = 1 ; b <= B; b++){
			oFile << "b:," << b << endl;
			for(c = 1 ; c <= C; c++){
				oFile << cplexSBAR.getValue(v[omega][p][b][c]) << ",";	
			}oFile << endl;
		}oFile << endl;
	}
	oFile << endl;
	oFile << endl;
}


oFile.close();


return 0;
}//end of main