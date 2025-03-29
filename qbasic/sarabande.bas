'=======================================================================
' Johann Sebastian Bach
' Sarabande from C Minor Cello Suite (BWV 1011)
' FreeBASIC version 20.09.2020
'=======================================================================
' More melodies here: https://github.com/vdust/beepy/tree/master/samples
' Info here: http://en.wikibooks.org/wiki/QBasic/Appendix#PLAY

dim as const string A = "O4 L8MLGE-<BMN>CL4<A- L8ML>>C<A-EMNFL4<B L8ML>>D<A-EMNFML<GMN>G MLFE-<BMN>CL4<C"
dim as const string B = "O4 L8MLCE-A-MNGML>D-MNC ML<DFB-MNA-ML>CMN<B- MLA-GDMNE-ML<B-MN>D L2ML<E-."
dim as const string C = "O4 L8MLB-GDMNE-L4<D- L8ML>B-GEMNFL4<G L8ML>>D-<B-EMNFML<CMN>>C ML<B-A-EMNFL4<F"
dim as const string D = "O3 L8MLE->CFMNE-MLB-MNA ML<D>DGMNFML>CMN<B ML>C<A-F+MNGML<BMN>C ML<G>DGMNF+ML>CMN<B"
dim as const string E = "O5 MLE-C<F+MNGML<AMN>>E- MLL64DE-L16D.L8<A-EMNFML<BMN>G MLFE-<BMN>CML<GMN>B ML<CG>FMNE-L4ML>C"

Play("T48" & A & B & A & B & C & D & E & C & D & E & "P2.")