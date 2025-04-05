
uses
  Play2Mid;

(*
========================================================================
 Johann Sebastian Bach
 Sarabande from C Minor Cello Suite (BWV 1011)
 FreeBASIC version 20.09.2020
========================================================================
*)

const
  A = 'O4 L8MLGE-<BMN>CL4<A- L8ML>>C<A-EMNFL4<B L8ML>>D<A-EMNFML<GMN>G MLFE-<BMN>CL4<C';
  B = 'O4 L8MLCE-A-MNGML>D-MNC ML<DFB-MNA-ML>CMN<B- MLA-GDMNE-ML<B-MN>D L2ML<E-.';
  C = 'O4 L8MLB-GDMNE-L4<D- L8ML>B-GEMNFL4<G L8ML>>D-<B-EMNFML<CMN>>C ML<B-A-EMNFL4<F';
  D = 'O3 L8MLE->CFMNE-MLB-MNA ML<D>DGMNFML>CMN<B ML>C<A-F+MNGML<BMN>C ML<G>DGMNF+ML>CMN<B';
  E = 'O5 MLE-C<F+MNGML<AMN>>E- MLL64DE-L16D.L8<A-EMNFML<BMN>G MLFE-<BMN>CML<GMN>B ML<CG>FMNE-L4ML>C';

  F = 't130 mb ml l4 p2 o2 e. l8 d c o1 b mn a4';
  G = 'a ml o2 c o1 b a g a e d mn e1';
  H = 'o2 e4. ml d c o1 b mn a4 a ml o2 c o1 b a g a e d e1';
  I = 'o2 mn d d d ml c o1 mn f4 f ml g o2 e4 d c o1 f4 a o2 c';
  J = 'o1 b4 a mn g g4 ml a g a1';

begin
  midiFileName := 'sarabande.mid';
{  Play('T48' + A + B + A + B + C + D + E + C + D + E + 'P2.', '');}
  Play(F + G + H + I + J, '');

end.
