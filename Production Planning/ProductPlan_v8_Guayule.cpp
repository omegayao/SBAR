//The following code is based on C++ and CPLEX 
//For the paper: Optimal Production Planning and Machinery Scheduling for Semi-Arid Farms


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
//
//--------------------read input data file ---------------------
char *strr=argv[1];
cout<<"The input file is "<<strr<<endl;
ifstream iData(strr);

assert(iData);

	int Version;			//Crops 

//Sets 
	int C;			//Crops  
	int I;			//Irrigation districts
	int MP; 		//Planting machinery 
	int P;			//Parcels
	int D;			//Distances between parcels
	int T;			//Time periods
	
	int c; 			//crops counter
	int i;			//irrigation district counter
	int m; 			//Planting machinery counter  
	int n; 			//Auxiliary counter 
	int p; 			//Parcel counter
	int q;			//Parcel auxiliary counter
	int d;			//Distance counter
	int t;			//Time periods counter
	int t2;			//Time periods Auxiliary counter
	int t3;			//Time periods Auxiliary counter


	//reading Sets inputs
	iData >> Version;
	iData >> C;
	iData >> I;	
	iData >> MP;						
	iData >> P;

	D = P * P * C;

	iData >> T;			
	
	//reading Economic Parameter inputs
	float D_ct[C+1][T+1];			//Demand
	float CCapPMt_c[C + 1][T+1];	//Capital Cost Machinery Planting
	float CPM_c[C + 1];				//Machinery Operational Cost Planting
	float CFH_c[C + 1];				//Fertilizer and Herbicice Cost
	float CTP_c[C + 1];				//Transportation Cost Planting Machinery 
	float CI;						//Irrigation Water Cost
	float CL;						//Irrigation Labor Cost
	float CS_c[C + 1];				//Seed cost
	float S_c[C + 1];				//Sale Price for Crop
	float IRInc;					//Sale Price Inflation Rate
	float IRCost;					//Costs Inflation Rate
	float Alpha;					//Discount Rate

	for (c = 1; c <= C; c++) {
		for (t = 1; t <= T; t++) {
			iData >> D_ct[c][t];
		}
	}

	for (c = 1; c <= C; c++) {
		for (t = 1; t <= T; t++) {
			iData >> CCapPMt_c[c][t];
		}
	}

	for (c = 1; c <= C; c++) {
		iData >> CPM_c[c];
	}

	for (c = 1; c <= C; c++) {
		iData >> CFH_c[c];
	}

	for (c = 1; c <= C; c++) {
		iData >> CTP_c[c];
	}

	iData >> CI;
	iData >> CL;
	
	for (c = 1; c <= C; c++) {
		iData >> CS_c[c];
	}

	for (c = 1; c <= C; c++) {
		iData >> S_c[c];
	}

	iData >> IRInc;
	iData >> IRCost;
	iData >> Alpha;

	//reading Harvesting Parameter inputs
	float P_i[P + 1];					//Irrigation district per parcel
	float T_c[C + 1];					//Time periods for crop C
	float NR_c[C + 1];					//Maximum sequential harvest of crop c before rotation
	float Y_cp[C + 1][P + 1];			//Yield of Crop per Parcel
	float A_cp[C + 1][P + 1];			//Acres of Crop per Parcel
	float P_cp[C + 1][P + 1];			//Productivity of Crop per Parcel
	float D_d[C + 1][P + 1][P + 1];		//Distance between parcels for crop
	float MCapP;						//Planting machinery capacity per lenght of harvest season
	float MaxPP;						//Maximum number of parcels to be visited by the Planting Macinery

	for (p = 1; p <= P; p++) {
		iData >> P_i[p];
	}

	for (c = 1; c <= C; c++) {
		iData >> T_c[c];
	}
	
	for (c = 1; c <= C; c++) {
		iData >>NR_c[c];
	}

	for (c = 1; c <= C; c++) {
		for (p = 1; p <= P; p++) {
			iData >> Y_cp[c][p];
		}
	}

	for (c = 1; c <= C; c++) {
		for (p = 1; p <= P; p++) {
			iData >> A_cp[c][p];
		}
	}

	for (c = 1; c <= C; c++) {
		for (p = 1; p <= P; p++) {
			iData >> P_cp[c][p];
		}
	}

	for (c = 1; c <= C; c++) {
		for (p = 1; p <= P; p++) {
			for (q = 1; q <= P; q++) {
				iData >> D_d[c][p][q];
			}
		}
	}

	iData >> MCapP;
	iData >> MaxPP;

	//reading Water Parameter inputs
	float W_ct[C + 1][T + 1];			//Irrigation water for Crop
	float WF_cp[C + 1][P + 1];			//Maximum water irrigation for crop at parcel
	float WI_i[I + 1];					//Water availability in Irrigation District

	for (c = 1; c <= C; c++) {
		for (t = 1; t <= T; t++) {
			iData >> W_ct[c][t];
		}
	}

	for (c = 1; c <= C; c++) {
		for (p = 1; p <= P; p++) {
			iData >> WF_cp[c][p];
		}
	}

	for (i = 1; i <= I; i++) {
		iData >> WI_i[i];
	}


iData.close();

//--------------------end of input data file--------------------
//--------------------------------------------------------------


//------------------CHECK INPUT DATA ---------------------------

cout << "---------------------------------------------- SETS ------------------------------------------------" << endl;
cout << "Crops: " << C << endl;
cout << "Irrigation districts: " << I << endl;
cout << "Planting Machinery: " << MP << endl;
cout << "Parcels: " << P << endl;
cout << "Distance between parcels: " << D << endl;
cout << "Time Periods: " << T << endl;

cout << "--------------------------------------- ECONOMIC PARAMETERS ------------------------------------------------" << endl;

cout << "Demand for crop in time period: " << endl;
for (c = 1; c <= C; c++) {
	for (t = 1; t <= T; t++) {
		cout << D_ct[c][t] << " ";
	}
	cout << endl;
}


cout << "Loan Amortization Planting Machinery: " << endl;
for (c = 1; c <= C; c++) {
	for (t = 1; t <= T; t++) {
		cout << CCapPMt_c[c][t] << " ";
	}cout << endl;
}
cout << endl;


cout << "Machinery Operational Cost Planting: " << endl;
for (c = 1; c <= C; c++) {
	cout << CPM_c[c] << " ";
}
cout << endl;


cout << "Fertilizer and Herbicice Cost: " << endl;
for (c = 1; c <= C; c++) {
	cout << CFH_c[c] << " ";
}
cout << endl;


cout << "Transportation Cost Planting Machinery: " << endl;
for (c = 1; c <= C; c++) {
	cout << CTP_c[c] << " ";
}
cout << endl;


cout << "Irrigation Water Cost: " << endl;
cout << CI << endl;

cout << "Irrigation Labor Cost: " << endl;
cout << CL << endl;

cout << "Seed cost: " << endl;
for (c = 1; c <= C; c++) {
	cout << CS_c[c] << " ";
}
cout << endl;


cout << "Sale Price for Crop: " << endl;
for (c = 1; c <= C; c++) {
	cout << S_c[c] << " ";
}
cout << endl;


cout << "Sale Price Inflation Rate: " << endl;
cout << IRInc << endl;

cout << "Costs Inflation Rate: " << endl;
cout << IRCost << endl;

cout << "Discount Rate: " << endl;
cout << Alpha << endl;

cout << "--------------------------------------- SCHEDULING PARAMETERS ------------------------------------------------" << endl;

cout << "Irrigation district per parcel: " << endl;
for (p = 1; p <= P; p++) {
	cout << P_i[p] << " ";
}
cout << endl;

cout << "Time periods for crop C: " << endl;
for (c = 1; c <= C; c++) {
	cout << T_c[c] << " ";
}
cout << endl;

cout << "Yield of Crop per Parcel: " << endl;
for (c = 1; c <= C; c++) {
	for (p = 1; p <= P; p++) {
		cout << Y_cp[c][p] << " ";
	}
	cout << endl;
}
cout << endl;

cout << "Acres of Parcel from crop C: " << endl;
for (c = 1; c <= C; c++) {
	for (p = 1; p <= P; p++) {
		cout << A_cp[c][p] << " ";
	}
	cout << endl;
}

cout << "Productivity for crop c: " << endl;
for (c = 1; c <= C; c++) {
	for (p = 1; p <= P; p++) {
		cout << P_cp[c][p] << " ";
	}
	cout << endl;
}
cout << endl;

cout << "Distance between parcels for crop: " << endl;
for (c = 1; c <= C; c++) {
	for (p = 1; p <= P; p++) {
		for (q = 1; q <= P; q++) {
			cout << D_d[c][p][q] << " ";
		}
		cout << endl;
	}
	cout << endl;
	cout << endl;
}

cout << "Capacity Planting Machinery: " << endl;
cout << MCapP << endl;

cout << "Maximum number of parcels per Planting Machinery: " << endl;
cout << MaxPP << endl;


cout << "--------------------------------------- WATER PARAMETERS ------------------------------------------------" << endl;

cout << "Irrigation water for Crop: " << endl;
for (c = 1; c <= C; c++) {
	for (t = 1; t <= T; t++) {
		cout << W_ct[c][t] << " ";
	}
	cout << endl;
}
cout << endl;

cout << "Maximum water irrigation for crop at parcel: " << endl;
for (c = 1; c <= C; c++) {
	for (p = 1; p <= P; p++) {
		cout << WF_cp[c][p] << " ";
	}
	cout << endl;
}
cout << endl;

cout << "Water availability in Irrigation District: " << endl;
for (i = 1; i <= I; i++) {
	cout << WI_i[i] << " ";
}
cout << endl;


//-------------------END OF CHECK INPUT DATA--------------------
//output file
char outputfilename[40];
sprintf(outputfilename,"SBARpaper8_AZ%d.csv",Version);
ofstream oFile(outputfilename);

//create the CPLEX variable
typedef IloArray<IloNumVarArray> IloNumVarArray2;
typedef IloArray<IloNumVarArray2> IloNumVarArray3;
typedef IloArray<IloNumVarArray3> IloNumVarArray4;

typedef IloArray<IloBoolVarArray> IloBoolVarArray2;
typedef IloArray<IloBoolVarArray2> IloBoolVarArray3;
typedef IloArray<IloBoolVarArray3> IloBoolVarArray4;
typedef IloArray<IloBoolVarArray4> IloBoolVarArray5;

typedef IloArray<IloIntVarArray> IloIntVarArray2;
typedef IloArray<IloIntVarArray2> IloIntVarArray3;

//Create the CPLEX environment 	
IloEnv env; 

	//ST Model creation
	IloModel SBARmodel(env);
   
	//VARIABLE DEFINITION

	//Planning - Planting Machinery
	IloBoolVarArray2 u(env, MP + 1);				//u[m][c]
	for (m = 1; m <= MP; m++) {
		u[m] = IloBoolVarArray(env, C + 1);
	}


//********** Added 1000 as upper bound to avoid using IloInfinity
	//Subtour variable for planting
	IloIntVarArray w(env, P + 1, 0, 1000);					//w[p]				

//***************************************************************
	//Crop variable
	IloBoolVarArray3 x(env, T + 1);				//x[t][c][p]
	for (t = 1; t <= T; t++) {
		x[t] = IloBoolVarArray2(env, C + 1);
		for (c = 1; c <= C; c++) {
			x[t][c] = IloBoolVarArray(env, P + 1);
		}
	}

	//Planting Machinery Assignation
	IloBoolVarArray4 y(env, T + 1);				//y[t][p][q][c]
	for (t = 1; t <= T; t++) {
		y[t] = IloBoolVarArray3(env, P + 1);
		for (p = 1; p <= P; p++) {
			y[t][p] = IloBoolVarArray2(env, P + 1);
			for (q = 1; q <= P; q++) {
				y[t][p][q] = IloBoolVarArray(env, C + 1);
			}
		}
	}

cout << "Variables created" << endl;	
//***********************************************************************************************************************	
	//Objective function
		IloExpr OBJ(env);
	
		//Revenue
		for (t = 1; t <= T; t++){
			for (c = 1; c <= C; c++) {
				for(p = 1; p <= P; p++) {
					if(t==1){
						OBJ += x[t][c][p] * P_cp[c][p] * S_c[c] * pow((1 + IRInc),t);
					}
					if(t>=1){
						OBJ += 1/pow((1+Alpha),(t-1)) * x[t][c][p] * P_cp[c][p] * S_c[c] * pow((1 + IRInc),t);
					}
				}
			}
			//TotaCost - Part 1 Loan Amortization Machinery
			for (c = 1; c <= C; c++) {
				for(m = 1; m <= MP; m++) {
					if(t==1){
						OBJ += - CCapPMt_c[c][t] * u[m][c]; 
					}
					if(t>=1){
						OBJ += - 1/pow((1+Alpha),(t-1)) * CCapPMt_c[c][t] * u[m][c]; 
					}					
				}
			}
			//TotaCost - Part 2 Machinery Operation
			for (c = 1; c <= C; c++) {
				for (p = 1; p <= P; p++) {
					if(t==1){
						OBJ += - (x[t][c][p] * A_cp[c][p] *  CPM_c[c])  * pow((1 + IRInc),t);
					}
					if(t>=1){
						OBJ += - 1/pow((1+Alpha),(t-1)) * (x[t][c][p] * A_cp[c][p] *  CPM_c[c])  * pow((1 + IRInc),t);
					}						
				}
			}
			//TotaCost - Part 3 Machinery Transportation
			for (p = 1; p <= P; p++) {
				for (q = 1; q <= P; q++) {
					for (c = 1; c <= C; c++) {
						if(t==1){
							OBJ += - (CTP_c[c] * D_d[c][p][q] * y[t][p][q][c]) * pow((1 + IRInc),t);
						}
						if(t>=1){
							OBJ += - 1/pow((1+Alpha),(t-1)) * (CTP_c[c] * D_d[c][p][q] * y[t][p][q][c]) * pow((1 + IRInc),t);
						}	
					}
				}
			}
			//TotaCost - Part 4 Other Costs
			for (c = 1; c <= C; c++) {
				for(p = 1; p <= P; p++) {
					if(t==1){
						OBJ += - x[t][c][p] * ( (CS_c[c] + CFH_c[c] + (CI * W_ct[c][t]) ) * A_cp[c][p]) * pow((1 + IRInc),t); 
					}
					if(t>=1){
						OBJ += - 1/pow((1+Alpha),(t-1)) * x[t][c][p] * ( (CS_c[c] + CFH_c[c] + (CI * W_ct[c][t]) + CL) * A_cp[c][p]) * pow((1 + IRInc),t); 
					}
				}
			}	
		}

		SBARmodel.add(IloMaximize(env, OBJ));

		cout << "Objective Created" << endl;	
		
//***********************************************************************************************************************	
	//Constraints

	// Constraint (4)
	for (t = 1; t <= T; t++){
		for(p = 1 ; p <= P; p++){
			IloExpr Const_4(env);
			for(c = 1 ; c <= C; c++){
				Const_4 += x[t][c][p];
			}
			SBARmodel.add(Const_4 == 1);												
			Const_4.end();
		}
	}

	// Constraint (5a)
	for(c = 1 ; c <= C; c++){
		for(p = 1 ; p <= P; p++){
			IloExpr Const_5a(env);
			Const_5a += T_c[c] * x[1][c][p];
			for (t = 1; t <= T_c[c] ; t++){
				Const_5a += - x[t][c][p];
			}
			SBARmodel.add( Const_5a <= 0 );	
			Const_5a.end();			
		}
	}

	// Constraint (5b)
	for(c = 1 ; c <= C; c++){
		for(p = 1 ; p <= P; p++){
			for (t = 2; t <= T - T_c[c] + 1; t++){
				IloExpr Const_5b(env);
				IloExpr SumUp5b(env);
				for(t2 = t; t2 <= t + T_c[c] - 1; t2++){
					SumUp5b += x[t2][c][p];
				}
				Const_5b = SumUp5b - (T_c[c]*(x[t][c][p] - x[t-1][c][p]));
				SBARmodel.add(Const_5b >= 0 );											
				SumUp5b.end();
				Const_5b.end();
			}
		}
	}


	// Constraint (5c)
	for(c = 1 ; c <= C; c++){
		for(p = 1 ; p <= P; p++){
			for (t = T - T_c[c] + 2 ; t <= T; t++){                 
				IloExpr Const_5c(env);
				for(t2 = t; t2 <= T; t2++){
					Const_5c += (x[t2][c][p] - (x[t][c][p] - x[t-1][c][p]));
				}
				SBARmodel.add(Const_5c >= 0 );										
				Const_5c.end();
			}
		}			
	}


	// Constraint (6)
	//for(c = 1 ; c <= C; c++){
	{ c=1;
		for(p = 1 ; p <= P; p++){
			for (t = 1; t <= T - (NR_c[c]*T_c[c]); t++){                 
				IloExpr Const_6(env);
				for(t2 = t; t2 <= t + (NR_c[c]*T_c[c]); t2++){
					Const_6 += x[t2][c][p];
				}
				SBARmodel.add(Const_6 <= NR_c[c] * T_c[c] );										
				Const_6.end();
			}
		}			
	}

	// Constraint (7)
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			IloExpr Const_7(env);
			for(p = 1 ; p <= P; p++){
				Const_7 += (P_cp[c][p] * x[t][c][p]);
			}
			SBARmodel.add( Const_7 >= D_ct[c][t]);
			Const_7.end();
		}
	}


	// Constraint (8)
	for(i = 1 ; i <= I; i++){
		for (t = 1; t <= T; t++){
			IloExpr Const_8(env);
			for(p = 1 ; p <= P; p++){
				for(c = 1 ; c <= C; c++){
					Const_8 += A_cp[c][p] * W_ct[c][t] * x[t][c][p];
				}
			}
			SBARmodel.add( Const_8 - WI_i[i] <= 0);
			Const_8.end();
		}
	}
	
	// Constraint (9)
	for(p = 1 ; p <= P; p++){
		for(c = 1 ; c <= C; c++){
			for (t = 1; t <= T; t++){
				SBARmodel.add( (A_cp[c][p] * W_ct[c][t] * x[t][c][p]) - WF_cp[c][p]<= 0);
			}
		}
	}

	// Constraint (10)
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			IloExpr Const_10(env);
			for(p = 1 ; p <= P; p++){
				Const_10 += (1/MCapP) * A_cp[c][p] * x[t][c][p];
			}
			for (m = 1; m <= MP; m++) {
				Const_10 += - u[m][c];
			}
			SBARmodel.add( Const_10 <= 0);
			Const_10.end();
		}
	}

	// Constraint (11)
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			IloExpr Const_12(env);
			for(q = 1 ; q <= P; q++){
				Const_12 += y[t][1][q][c];
			}
			for (m = 1; m <= MP; m++) {
				Const_12 += - u[m][c];
			}
			SBARmodel.add( Const_12 == 0);
			Const_12.end();
		}
	}
	
		// Constraint (12)
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			IloExpr Const_14(env);
			for(p = 1 ; p <= P; p++){
				Const_14 += y[t][p][1][c];
			}
			for (m = 1; m <= MP; m++) {
				Const_14 += - u[m][c];
			}
			SBARmodel.add( Const_14 == 0);
			Const_14.end();
		}
	}
	
	
	// Constraint (13)	
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			for(p = 2 ; p <= P; p++){
				IloExpr Const_16(env);
				for(q = 1 ; q <= P; q++){
					if (q != p){
						Const_16 += y[t][q][p][c];
					}
				}
				SBARmodel.add( Const_16 - x[t][c][p] == 0);
				Const_16.end();
			}
		}
	}
		

	// Constraint (14)	
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			for(p = 2 ; p <= P; p++){
				IloExpr Const_18(env);
				for(q = 1 ; q <= P; q++){
					if (q != p){
						Const_18 += y[t][p][q][c];
					}
				}
				SBARmodel.add( Const_18 - x[t][c][p] == 0);
				Const_18.end();
			}
		}
	}
	
	// Constraint (15)	
	for(c = 1 ; c <= C; c++){
		for (t = 1; t <= T; t++){
			for(q = 2 ; q <= P; q++){
				for(p = 2 ; p <= P; p++){
					if (q != p){
						SBARmodel.add( w[p] - w[q] + (MaxPP * y[t][p][q][c]) - (MaxPP - 1) <= 0);
					}
				}
			}
		}
	}
	
	
//*****************************************************************************************	
//-------------------------------------- CPLEX Models -------------------------------------
//*****************************************************************************************	

float Time;					//Parameters for recording time for Second-Third level
Time = 0;
float starttime; 


IloCplex cplexSBAR(SBARmodel);
IloBool successFS = false;
IloAlgorithm::Status status_FS;

//Export Model
//cplexSBAR.exportModel("SBAR_Paper8_NM_ver.lp");

//set gap parameters
cplexSBAR.setParam(IloCplex::EpGap,0.02);//gap 
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
//CALCULATE OBJECTIVE BREAKDOWN PER YEAR WITHOUT DISCOUNT (NO NPV)

float Revenuew[C+1][T+1], TotalCost[C+1][T+1], SeedCost[C+1][T+1], FerHerCost[C+1][T+1], WaterCost[C+1][T+1];
float MachineryCost[C+1][T+1], OperationalCost[C+1][T+1], TransportationCost[C+1][T+1];

for (c = 1; c <= C; c++) {
	for (t = 1; t <= T; t++){
		Revenuew[c][t] = 0; 
		TotalCost[c][t] = 0; 
		SeedCost[c][t] = 0;
		FerHerCost[c][t] = 0;
		WaterCost[c][t] = 0;
		MachineryCost[c][t] = 0;
		OperationalCost[c][t] = 0;
		TransportationCost[c][t] = 0;
	}
}


for (t = 1; t <= T; t++){
	for (c = 1; c <= C; c++) {
		for(p = 1; p <= P; p++) {
			Revenuew[c][t] +=  cplexSBAR.getValue(x[t][c][p]) * P_cp[c][p] * S_c[c] * pow((1 + IRInc),t);
		}
	}

	//TotaCost - Part 1 Loan Amortization Machinery
	for (c = 1; c <= C; c++) {
		for(m = 1; m <= MP; m++) {
			TotalCost[c][t] += CCapPMt_c[c][t] * cplexSBAR.getValue(u[m][c]); 
			MachineryCost[c][t] += CCapPMt_c[c][t] * cplexSBAR.getValue(u[m][c]); 
		}
	}
	//TotaCost - Part 2 Machinery Operation
	for (c = 1; c <= C; c++) {
		for (p = 1; p <= P; p++) {
			TotalCost[c][t] +=  cplexSBAR.getValue(x[t][c][p]) * A_cp[c][p] * CPM_c[c] * pow((1 + IRInc),t);
			OperationalCost[c][t] +=  cplexSBAR.getValue(x[t][c][p]) * A_cp[c][p] * CPM_c[c] * pow((1 + IRInc),t);
		}
	}
	//TotaCost - Part 3 Machinery Transportation
	for (p = 1; p <= P; p++) {
		for (q = 1; q <= P; q++) {
			for (c = 1; c <= C; c++) {
				if (q != p){
					TotalCost[c][t] += (CTP_c[c] * D_d[c][p][q] * cplexSBAR.getValue(y[t][p][q][c]) ) * pow((1 + IRInc),t);
					TransportationCost[c][t] += (CTP_c[c] * D_d[c][p][q] * cplexSBAR.getValue(y[t][p][q][c]) ) * pow((1 + IRInc),t);
				}
			}
		}
	}
	//TotaCost - Part 4 Other Costs
	for (c = 1; c <= C; c++) {
		for(p = 1; p <= P; p++) {
			TotalCost[c][t] += cplexSBAR.getValue(x[t][c][p]) * ( (CS_c[c] + CFH_c[c] + (CI * W_ct[c][t]) ) * A_cp[c][p]) * pow((1 + IRInc),t); 
			SeedCost[c][t] += cplexSBAR.getValue(x[t][c][p]) * CS_c[c] * A_cp[c][p]* pow((1 + IRInc),t);
			FerHerCost[c][t] += cplexSBAR.getValue(x[t][c][p]) * CFH_c[c] * A_cp[c][p] * pow((1 + IRInc),t);
			WaterCost[c][t] += cplexSBAR.getValue(x[t][c][p]) * CI * W_ct[c][t] * A_cp[c][p] * pow((1 + IRInc),t);
		} 
	}	
}

oFile << "Optimal Objective:," << cplexSBAR.getValue(OBJ) << endl;
cout << "Optimal Objective: " << cplexSBAR.getValue(OBJ) << endl;

oFile << "Time:," << Time << endl;

for (c = 1; c <= C; c++) {
	oFile << "Crop: " << c << endl;
	oFile << "Year:,";
	for (t = 1; t <= T; t++){
		oFile << t << ",";
	}oFile << endl;

	oFile << "Revenue:,";
	for (t = 1; t <= T; t++){
		oFile << Revenuew[c][t] << ",";
	}oFile << endl;

	oFile << "Total Cost:,";
	for (t = 1; t <= T; t++){
		oFile << TotalCost[c][t] << ",";
	}oFile << endl;

	oFile << "Seed Cost:,";
	for (t = 1; t <= T; t++){
		oFile << SeedCost[c][t] << ",";
	}oFile << endl;

	oFile << "Fertilizer and Herbicides Costs:,";
	for (t = 1; t <= T; t++){
		oFile << FerHerCost[c][t] << ",";
	}oFile << endl;

	oFile << "Water Cost:,";
	for (t = 1; t <= T; t++){
		oFile << WaterCost[c][t] << ",";
	}oFile << endl;

	oFile << "Loan Amortization Machinery Cost:,";
	for (t = 1; t <= T; t++){
		oFile << MachineryCost[c][t] << ",";
	}oFile << endl;

	oFile << "Operational Cost:,";
	for (t = 1; t <= T; t++){
		oFile << OperationalCost[c][t] << ",";
	}oFile << endl;
	oFile << endl;

	oFile << "Transportation Cost:,";
	for (t = 1; t <= T; t++){
		oFile << TransportationCost[c][t] << ",";
	}oFile << endl;
	oFile << endl;
}

// Printing Variables

oFile << "Planting Machinery, u[m][c]:" << endl;
for (c = 1; c <= C; c++) {
	oFile << "c:," << c << endl;
	for (m = 1; m <= MP; m++) {
		oFile << cplexSBAR.getValue(u[m][c]) << ",";	
	}oFile << endl;
}
oFile << endl;
oFile << endl;

oFile << "Crop Decision, x[t][c][p]:" << endl;
for (c = 1; c <= C; c++) {
	oFile << "c:," << c << ", p: ,";
	for (p = 1; p <= P; p++) {
			oFile << p << ",";
	}oFile << endl;
	for (t = 1; t <= T; t++) {
		oFile << "t:," << t << ", Parcels:,";
		for (p = 1; p <= P; p++) {
			oFile << cplexSBAR.getValue(x[t][c][p]) << ",";
		}oFile << endl;
	}oFile << endl;
}
oFile << endl;
oFile << endl;

oFile << "Planting Machinery Assignation, y[t][p][q][c]:" << endl;
for (t = 1; t <= T; t++) {
	oFile << "t:," << t << endl;
	for (c = 1; c <= C; c++) {
		oFile << "c:," << c << endl;
		for (p = 1; p <= P; p++) {
			oFile << "Parcel:,";
			if(p == 1){
				for (n = 1; n <= P; n++) {   //n used as auxiliary counter here
					oFile << n << ",";
				}oFile << endl;
			}
			oFile << p << ",";
			for (q = 1; q <= P; q++) {
				if (q != p){
					oFile << cplexSBAR.getValue(y[t][p][q][c]) << ",";
				}else{
					oFile << "-" << ",";
				}
			}oFile << endl;
		}oFile << endl;
	}oFile << endl;
}
oFile << endl;
oFile << endl;


oFile.close();


return 0;
}//end of main