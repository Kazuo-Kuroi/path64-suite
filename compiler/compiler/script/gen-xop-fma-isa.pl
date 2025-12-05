#!/usr/bin/perl

###############################################################################################################
#this script is used to generate the isa of XOP, FMA, AES
#Author: Yiming Lei
###############################################################################################################


use Data::Dumper;

sub fprint{
        my($filename,$content)=@_;
        return if not ($content);
        open(FILE, ">".$filename) or die "can't open file: $filename\n";
        print FILE $content;
        close FILE;
}

sub cartesian {		#cartesian product, the perl way ]:->
    my @C=[];
    foreach(reverse @_){
        my @A=@$_;
        @C=map{my $n=$_;map{[$n,@$_]} @C} @A;
    }
    return @C;
}
sub access_memory{
	my($top_instructions)=@_;
	if($top_instructions=~/mem/){
		#print "hehe met memory".$top_instructions."\n";
	  return 1;
	}else{
	  return 0;
	}
}


sub type_operation{
	my($top_instr)=@_;
	if($top_instr=~/omem/){ return "store_only";}
	elsif($top_instr=~/mem/){
	  #print "load  ".$top_instr."\n";
	  if($top_instr=~/mov/){
	    return "load_only";
  	  }else{
	    return "load_exe";
	  }
  }
	else{
	  #print "else fuck".$top_instr."\n";
	  if($top_instr=~/mov/){
	    return "move_prop";
   }elsif($top_instr=~/add/){
	    return "fadd_prop";
	  }elsif($top_instr=~/sub/){
	    return "fsub_prop";
	  }elsif($top_instr=~/mul/){
	    return "fmul_prop";
	  }
		else{
	    return "none_prop";
	  }
	}
}

sub copy_to{
	my($path,$filename)=@_;
	return if not ($filename);
	system("mv -f $filename $path");
}

sub op_mode_match{
  my($reg,$mem)=@_;
		print "shiiiiiiiiiiiiiiiiiiiiiiit";
		print $reg."\n";
		print $mem."\n";
		print "shiiiiiiiiiiiiiiiiiiiiiiit";
		my @tmp_s_reg;
		my @tmp_s_mem;
		@tmp_s_reg=split("_o",$reg);
		@tmp_s_mem=split("_o",$mem);
		print "\nfffdd ".$tmp_s_reg[1]." fff\n";
		print "\ndddff ".$tmp_s_mem[1]." ddd\n";
		my @tmp_reg;
		my @tmp_mem;
		@tmp_reg=split("_",$tmp_s_reg[1]);
		@tmp_mem=split("_",$tmp_s_mem[1]);
		print "\nfff ".$tmp_reg[1]." fff\n";
		print "\nddd ".$tmp_mem[1]." ddd\n";
		if(scalar(@tmp_reg)!=scalar(@tmp_mem)){
		  print "hehehehhheh";
		  return 0;
		}else{
		  my $i;
				my $different = 0;
				for($i=0;$i<scalar(@tmp_reg);$i++){
				  if(!($tmp_reg[$i]=~/^$tmp_mem[$i]$/)){
						  print $tmp_reg[$i]." match ".$tmp_mem[$i]."\n";
						  $different++;
						}
				}
				print "different is ".$different."\n";
				return $different;
		}
}

sub how_many_opnds{
	my($memory_op)=@_;
	if($memory_op eq ""){
	  return 0;
	}
	if($memory_op eq "mem1"){ return 2;}
 elsif($memory_op eq "mem2"){return 4;}
 elsif($memory_op eq "mem3"){ return 3;}
 else { return 1;}
	#my @mem;
	#@mem=split("_",$memory_op);
	#return scalar(@mem);

}

#pulling things out of envytools and converting them directly to isa_* interface results in fugly names, but at least it's fast as hell.

#my @mem=[["base64"],["offset32"],[["base64"],["offset64"],["uimm8"],["simm32"]],[["offset64"],["uimm8"],["simm32"]];
#my @mem =["base64","offset32"];
#################################################################################################
#mem1 is base_mode
#mem2 is base_index_mode
#mem3 is index_mode
#mem4 is n32_mode
my @mem=["mem1","mem2","mem3"];
my @omem=["omem1","omem2","omem3"];
#################################################################################################
my @oxmm_xmm_xmm_xmm=["vfmaddpd", "vfmaddps","vfmaddsd","vfmaddss","vfmaddsubpd","vfmaddsubps","vfmsubaddpd","vfmsubaddps","vfmsubpd","vfmsubps","vfmsubsd","vfmsubss","vfnmaddpd","vfnmaddps","vfnmaddsd","vfnmaddss","vfnmsubpd","vfnmsubps","vfnmsubsd","vfnmsubss","vpcmov","vpmacsdd","vpmacsdqh","vpmacsdql","vpmacssdd","vpmacssdqh","vpmacssdql","vpmacsswd","vpmacssww","vpmacswd","vpmacsww","vpmadcsswd","vpmadcswd","vpperm"];
my @oxmm_xmm_mem_xmm=["vfmaddpd", "vfmaddps","vfmaddsd","vfmaddss","vfmaddsubpd","vfmaddsubps","vfmsubaddpd","vfmsubaddps","vfmsubpd","vfmsubps","vfmsubsd","vfmsubss","vfnmaddpd","vfnmaddps","vfnmaddsd","vfnmaddss","vfnmsubpd","vfnmsubps","vfnmsubsd","vfnmsubss","vpcmov","vpmacsdd","vpmacsdqh","vpmacsdql","vpmacssdd","vpmacssdqh","vpmacssdql","vpmacsswd","vpmacssww","vpmacswd","vpmacsww","vpmadcsswd","vpmadcswd","vpperm"];
my @oxmm_xmm_xmm_mem=["vfmaddpd", "vfmaddps","vfmaddsd","vfmaddss","vfmaddsubpd","vfmaddsubps","vfmsubaddpd","vfmsubaddps","vfmsubpd","vfmsubps","vfmsubsd","vfmsubss","vfnmaddpd","vfnmaddps","vfnmaddsd","vfnmaddss","vfnmsubpd","vfnmsubps","vfnmsubsd","vfnmsubss","vpcmov","vpperm"];
my @oymm_ymm_ymm_ymm=["vfmaddpd", "vfmaddps","vfmaddsubpd","vfmaddsubps","vfmsubaddpd","vfmsubaddps","vfmsubpd","vfmsubps","vfnmaddpd","vfnmaddps","vfnmaddsd","vfnmsubpd","vfnmsubps","vpcmov"];
my @oymm_ymm_mem_ymm=["vfmaddpd", "vfmaddps","vfmaddsubpd","vfmaddsubps","vfmsubaddpd","vfmsubaddps","vfmsubpd","vfmsubps","vfnmaddpd","vfnmaddps","vfnmaddsd","vfnmsubpd","vfnmsubps","vpcmov"];
my @oymm_ymm_ymm_mem=["vfmaddpd", "vfmaddps","vfmaddsubpd","vfmaddsubps","vfmsubaddpd","vfmsubaddps","vfmsubpd","vfmsubps","vfnmaddpd","vfnmaddps","vfnmaddsd","vfnmsubpd","vfnmsubps","vpcmov"];
my @oxmm_xmm=["vfrczpd", "vfrczps","vfrczsd","vfrczss","vphaddbd","vphaddbq","vphaddbw","vphadddq","vphaddwd","vphaddwq","vphaddubd","vphaddubq","vphaddubw","vphaddudq","vphadduwd","vphadduwq","vphsubbw","vphsubdq","vphsubwd","aesdec","aesenclast","aesenc","aesdeclast","aesimc"];
my @oxmm_mem=["vfrczpd", "vfrczps","vfrczsd","vfrczss","vphaddbd","vphaddbq","vphaddbw","vphadddq","vphaddwd","vphaddwq","vphaddubd","vphaddubq","vphaddubw","vphaddudq","vphadduwd","vphadduwq","vphsubbw","vphsubdq","vphsubwd","aesdec","aesenclast","aesenc","aesdeclast","aesimc"];
my @oymm_ymm=["vfrczpd", "vfrczps"];
my @oymm_mem=["vfrczpd", "vfrczps"];
my @oxmm_xmm_xmm_simm8=["vpcomb","vpcomd","vpcomq","vpcomub","vpcomud","vpcomuq","vpcomuw","vpcomw"];
my @oxmm_xmm_mem_simm8=["vpcomb","vpcomd","vpcomq","vpcomub","vpcomud","vpcomuq","vpcomuw","vpcomw"];
my @oxmm_xmm_xmm_xmm_simm8=["vpermil2pd","vpermil2ps"];
my @oxmm_xmm_mem_xmm_simm8=["vpermil2pd","vpermil2ps"];
my @oxmm_xmm_xmm_mem_simm8=["vpermil2pd","vpermil2ps"];
my @oymm_ymm_ymm_ymm_simm8=["vpermil2pd","vpermil2ps"];
my @oymm_ymm_mem_ymm_simm8=["vpermil2pd","vpermil2ps"];
my @oymm_ymm_ymm_mem_simm8=["vpermil2pd","vpermil2ps"];
my @oxmm_xmm_xmm=["vprotb","vprotd","vprotq","vprotw","vpshab","vpshad","vpshaq","vpshaw","vpshlb","vpshld","vpshlq","vpshlw"];
my @oxmm_mem_xmm=["vprotb","vprotd","vprotq","vprotw","vpshab","vpshad","vpshaq","vpshaw","vpshlb","vpshld","vpshlq","vpshlw"];
my @oxmm_xmm_mem=["vprotb","vprotd","vprotq","vprotw","vpshab","vpshad","vpshaq","vpshaw","vpshlb","vpshld","vpshlq","vpshlw"];
my @oxmm_xmm_simm8=["vprotb","vprotd","vprotq","vprotw","aeskeygenassist","pclmulqdq"];
my @oxmm_mem_simm8=["vprotb","vprotd","vprotq","vprotw","aeskeygenassist","pclmulqdq"];
##########################################################################################

my @ops=(
	[@oxmm_xmm,["f128"],["OPS"],["oxmm"],["xmm"]],
	[@oxmm_mem,["f128"],["OPS"],["oxmm"],@mem],
	[@oymm_ymm,["f256"],["OPS"],["oymm"],["ymm"]],
	[@oymm_mem,["f128"],["OPS"],["oymm"],@mem],
	[@oxmm_xmm_xmm,["f128"],["OPS"],["oxmm"],["xmm"],["xmm"]],
 [@oxmm_mem_xmm,["f128"],["OPS"],["oxmm"],@mem,["xmm"]],
	[@oxmm_xmm_mem,["f128"],["OPS"],["oxmm"],["xmm"],@mem],
	[@oxmm_xmm_simm8,["f128"],["OPS"],["oxmm"],["xmm"],["simm8"]],
	[@oxmm_mem_simm8,["f128"],["OPS"],["oxmm"],@mem,["simm8"]],
	[@oxmm_xmm_xmm_xmm,["f128"],["OPS"],["oxmm"],["xmm"],["xmm"],["xmm"]],
	[@oxmm_xmm_xmm_mem,["f128"],["OPS"],["oxmm"],["xmm"],["xmm"],@mem],
	[@oxmm_xmm_mem_xmm,["f128"],["OPS"],["oxmm"],["xmm"],@mem,["xmm"]],
	[@oymm_ymm_ymm_ymm,["f256"],["OPS"],["oymm"],["ymm"],["ymm"],["ymm"]],
	[@oymm_ymm_ymm_mem,["f256"],["OPS"],["oymm"],["ymm"],["ymm"],@mem],
	[@oymm_ymm_mem_ymm,["f256"],["OPS"],["oymm"],["ymm"],@mem,["ymm"]],
	[@oxmm_xmm_xmm_simm8,["f128"],["OPS"],["oxmm"],["xmm"],["xmm"],["simm8"]],
	[@oxmm_xmm_mem_simm8,["f128"],["OPS"],["oxmm"],["xmm"],@mem,["simm8"]],
	[@oxmm_xmm_xmm_xmm_simm8,["f128"],["OPS"],["oxmm"],["xmm"],["xmm"],["xmm"],["simm8"]],
	[@oxmm_xmm_mem_xmm_simm8,["f128"],["OPS"],["oxmm"],["xmm"],@mem,["xmm"],["simm8"]],
	[@oxmm_xmm_xmm_mem_simm8,["f128"],["OPS"],["oxmm"],["xmm"],["xmm"],@mem,["simm8"]],
	[@oymm_ymm_ymm_ymm_simm8,["f256"],["OPS"],["oymm"],["ymm"],["ymm"],["ymm"],["simm8"]],
	[@oymm_ymm_mem_ymm_simm8,["f256"],["OPS"],["oymm"],["ymm"],@mem,["ymm"],["simm8"]],
	[@oymm_ymm_ymm_mem_simm8,["f256"],["OPS"],["oymm"],["ymm"],["ymm"],@mem,["simm8"]],
	);
my @isa;
my @tops; 		#for isa pack and subset
my %isa_operands;
my %isa_print;
my %ebo_special;
foreach $op (@ops){
	my @variants=cartesian(@$op);
	foreach(@variants){
		my $opstring;
		my @operands;
		my $op_pattern;
		my $op_prefix;
		my $isop=0;
		foreach (@$_){
			if($_ eq "OPS"){
				$isop=1;
			}
			elsif(not $_ eq ""){
				$opstring.=$_."_";
				push(@operands,$_) if $isop;
				$op_pattern.=$_."_" if $isop;
				$op_prefix.=$_."_" if not $isop;
			}
		}
		chop($op_pattern);
		chop($op_prefix);
		chop($opstring);
		my $topstring="TOP_".$opstring;
		push @isa,$opstring;
		push @tops,$topstring;
		push @{$isa_operands{$op_pattern}},[$topstring,\@operands];
		push @{$isa_print{$op_pattern}},$topstring;
		push @{$ebo_special{$op_prefix}},$topstring;
	}
}
my $isa_isa_print;
my $isa_operands_print;
my $isa_print_print;
my $isa_subset_print;
my $isa_pack_print;
my $isa_properties_print;
my $isa_si_print;
my $isa_cgemit_xop_fma_print;

my @debug_dup_isa;
my $di=0;

my $ebo_print;
foreach(keys %ebo_special){
  my @ebo_top=@{$ebo_special{$_}};
		print $_."has\n";
		my $op;
		my $top_count = scalar(@ebo_top);
		my @top_reg;
		my @top_mem1;
		my @top_mem2;
		my @top_mem3;
		foreach(@ebo_top){
		  print $_."\n";
				if($_=~/mem1/){
				  push @top_mem1,$_;
				}elsif($_=~/mem2/){
				  push @top_mem2,$_;
				}elsif($_=~/mem3/){
				  push @top_mem3,$_;
				}elsif($_=~/mem4/){
				  die "doesn't support for the moment";
				}else{
				  push @top_reg,$_;
				}
		}
		if(scalar(@top_reg)>1){
		  if(scalar(@top_mem1)!=scalar(@top_mem2) 
				  ||scalar(@top_mem2)!=scalar(@top_mem3)
						||scalar(@top_mem3)!=scalar(@top_mem1)){
						  die "memory addressing problem";
						}
		  #for($i=0; $i<scalar(@top_reg); $i++){
				#		my $j;
				#		my $match_value = 0;
				#		for($j=0;$j<scalar(@top_mem1);$j++){
				#		  $match_value=op_mode_match($top_reg[$i],$top_mem1[$j]);
				#		  if($match_value==1){
				#				   print "1 break;\n";
				#							break;
				#				}elsif($match_value==2){
				#				   print "2 break;\n";
				#				   break;
				#				}
				#		}
				#		if($match_value==2){next;}
				#		if($j==scalar(@top_mem1)){
				#		  die "mem1 doesn't have the mode of".$top_reg[$i]."j is".$j 
				#		}
				#  $ebo_print.="\t{".$top_reg[$i];
				#		$ebo_print.=",".$top_mem1[$j];
				#		$ebo_print.=",".$top_mem2[$j];
				#		$ebo_print.=",".$top_mem3[$j];
				#		$ebo_print.=",TOP_UNDEFINED".'},'."\n";
				#}
		}else{
		  if(scalar(@top_mem1)!=scalar(@top_mem2) 
				  ||scalar(@top_mem2)!=scalar(@top_mem3)
						||scalar(@top_mem3)!=scalar(@top_mem1)){
						  die "memory addressing problem";
						}
				my $i;
				for($i=0;$i<scalar(@top_mem1);$i++){
						if(scalar(@top_reg)==0){
				    $ebo_print.="\t{TOP_UNDEFINED";
						}else{$ebo_print.="\t{".$top_reg[0];}
						$ebo_print.=",".$top_mem1[$i];
						$ebo_print.=",".$top_mem2[$i];
						$ebo_print.=",".$top_mem3[$i];
						$ebo_print.=",TOP_UNDEFINED".'},'."\n";
				}
		}
}
print $ebo_print;

print "isa.cxx:\n";
foreach(@isa){
	$isa_isa_print.="\t\"".$_."\",\n";
	$debug_dup_isa[$di++]=$_;
}
if(1){
print "di = $di\n";

my $dj;
my $dk;
for($dj=0;$dj<$di;$dj++){
	#print "dj=$dj di=$di\n";
  for($dk=0;$dk<$di;$dk++){
    if($dj!=$dk){
      if($debug_dup_isa[$dk]=~/^$debug_dup_isa[$dj]$/i){
	      die "shit met duplicate:\n".$debug_dup_isa[$dk]."   ".$debug_dup_isa[$dj]."   dk=$dk  dj=$dj\n";
      }
    }
  }
  #print "shit ".$debug_dup_isa[$dj]."\n";
}
}
#$isa_print="\nisa_operands.cxx:\n";
foreach (keys %isa_operands){
	$isa_operands_print.="\tInstruction_Group(\"".$_."\",\n";
	foreach (@{$isa_operands{$_}}){
		$isa_operands_print.="\t\t".$_->[0].",\n";
	}
	$isa_operands_print.="\t\tTOP_UNDEFINED);\n";
	my $res=0;
	my $opnd=0;
	my $opnd_post=1;
	if($_=~/^omem/){
	  die "fma/xop dosent have store currently";
	}else{
	  foreach(@{$isa_operands{$_}[0]->[1]}){
		  if($_=~/^oxmm/||$_=~/^oymm/) {$isa_operands_print.="\tResult(".$res++.","."fp128".");\n";}
			if($_=~/^omem/) {die "no omem currently";}
			if($_=~/^xmm/||$_=~/^ymm/) {$isa_operands_print.="\tOperand(".$opnd++.", "."fp128".",opnd".$opnd_post++.");\n";}
			if($_=~/^simm8/){$isa_operands_print.="\tOperand(".$opnd++.", "."simm8".",opnd".$opnd_post++.");\n";}
			if($_=~/^mem1/){
				##base_mode
				$isa_operands_print.="\tOperand(".$opnd++.", "."int64".",base".");\n";
			  $isa_operands_print.="\tOperand(".$opnd++.", "."simm32".",offset".");\n";
				$opnd_post++;
		  }elsif($_=~/^mem2/){
        #base_index
				$isa_operands_print.="\tOperand(".$opnd++.", "."int64".",base".");\n";
				$isa_operands_print.="\tOperand(".$opnd++.", "."int64".",index".");\n";
				$isa_operands_print.="\tOperand(".$opnd++.", "."uimm8".",scale".");\n";
				$isa_operands_print.="\tOperand(".$opnd++.", "."simm32".",offset".");\n";
				$opnd_post++;
			}elsif($_=~/^mem3/){
				$isa_operands_print.="\tOperand(".$opnd++.", "."int64".",index".");\n";
				$isa_operands_print.="\tOperand(".$opnd++.", "."uimm8".",scale".");\n";
				$isa_operands_print.="\tOperand(".$opnd++.", "."simm32".",offset".");\n";
				$opnd_post++;
			}
			
		}
	}
	$isa_operands_print.="\n";

}
print "isa_print.cxx:\n";
foreach (keys %isa_operands){
	my $result_n=0;
	my $opnd_n=0;
	my @res;
	my @opnd;
	$isa_print_print.="ISA_PRINT_TYPE ".$_." = ISA_Print_Type_Create("."\"".$_."\",\n";
	foreach (@{$isa_operands{$_}[0]->[1]}){
		if($_=~/^o/){
		 $res[$result_n++]=$_;
		}else{
		 $opnd[$opnd_n++]=$_;
		}
	}

##the %s 
	$isa_print_print.='"%s ';
	my $i;
	for($i=$opnd_n-1;$i>=0;$i--){
	  if($opnd[$i]=~/^mem1$/){
		  $isa_print_print.='%s%s(%s) ';
		}elsif($opnd[$i]=~/^mem2$/){
		  $isa_print_print.='%s%s(%s,%s,%s) ';
		}elsif($opnd[$i]=~/^mem3$/){
			$isa_print_print.='%s%s(,%s,%s) ';
		}elsif($opnd[$i]=~/^(x|y)mm/||$opnd[$i]=~/^simm8$/){
		  $isa_print_print.='%s ';
		}elsif($opnd[$i]=~/^null/||$opnd[$i]=~/^mxcsr/){
		 ##do nothing here;
		}
		else{
		  $isa_print_print.="TODO NYI format opnd ".$opnd[$i]."\n";
		  die "TODO NYI format opnd ".$opnd[$i]."\n";
		}
		if($result_n>0){
			if($i!=0){##print , anyway
			 $isa_print_print.=',';
			}else{
			 if(!($res[0]=~/rflag/)){
			   $isa_print_print.=',';
			 }
			}
		}
	}
	my $store_op;
	$store_op=0;

	if($result_n>0){
	  if($res[0]=~/^omem1$/){
		  $isa_print_print.='%s%s(%s) ';
			$store_op=1;
		}elsif($res[0]=~/^omem2$/){
		  $isa_print_print.='%s%s(%s,%s,%s) ';
			$store_op=1;
		}elsif($res[0]=~/^omem3$/){
			$isa_print_print.='%s%s(,%s,%s) ';
			$store_op=1;
		}elsif($res[0]=~/^osimm8$/){ $isa_print_print.='%s';}
		elsif($res[0]=~/^o(x|y)mm/){ $isa_print_print.='%s'; }
		else{
		  $isa_print_print.="TODO NYI format result".$res[0]."\n";
				die "TODO NYI format result";
		}
	}
	$isa_print_print.="\");\n";

###end %s

###Operand()
###Result()
  my $total_opnd=0;
	$isa_print_print.="Name();\n"; 
	#for($i=0;$i<$opnd_n;$i++){
	for($i=$opnd_n-1;$i>=0;$i--){
	  if($opnd[$i]=~/^mem1$/){
			my $first;
			my $second;
			$first=$i+1;
			$second=$i;
			$isa_print_print.="Segment();\n";
			$isa_print_print.="Operand(".$first.");\n";
			$isa_print_print.="Operand(".$second.");\n";
		  #$isa_print_print.='%s%s(%s) ';
			$total_opnd+=2;
		}elsif($opnd[$i]=~/^mem2$/){
			my $first;
			my $second;
			my $third;
			my $fourth;
			$first=$i+3;
			$second=$i;
			$third=$i+1;
			$fourth=$i+2;
			$isa_print_print.="Segment();\n";
			$isa_print_print.="Operand(".$first.");\n";
			$isa_print_print.="Operand(".$second.");\n";
			$isa_print_print.="Operand(".$third.");\n";
			$isa_print_print.="Operand(".$fourth.");\n";

		  #$isa_print_print.='%s%s(%s,%s,%s) ';
			$total_opnd+=4;
		}elsif($opnd[$i]=~/^mem3$/){
			my $first;
			my $second;
			my $third;
			$first=$i+2;
			$second=$i;
			$third=$i+1;
			$isa_print_print.="Segment();\n";
			$isa_print_print.="Operand(".$first.");\n";
			$isa_print_print.="Operand(".$second.");\n";
			$isa_print_print.="Operand(".$third.");\n";
			#$isa_print_print.='%s%s(,%s,%s) ';
			$total_opnd+=3;
		}elsif($opnd[$i]=~/^simm8$/ || $opnd[$i]=~/^(x|y)mm/ || $opnd[$i]=~/^int/){
		  #$isa_print_print.='%s ';
		  	my $j,$number_opnd_front,$tmp_n;
			$number_opnd_front=0;
			for($j=0;$j<$i;$j++){
			  #determine how many opnds in front;
			  $number_opnd_front+=how_many_opnds($opnd[$j]);
			}
			$isa_print_print.="Operand(".$number_opnd_front.");\n";
		}elsif($opnd[$i]=~/^null/||$opnd[$i]=~/^mxcsr/){
		   #do nothing here;
		}
		else{
		  $isa_print_print.="TODO NYI $isa_print_print.=opnd ".$opnd[$i]."\n";
		}
	}
	
	if($result_n>0){
	  if($res[0]=~/^omem1$/){
			my $first;
			my $second;
			$first=$opnd_n+1;
			$second=$opnd_n;
			$isa_print_print.="Segment();\n";
			$isa_print_print.="Operand(".$first.");\n";
			$isa_print_print.="Operand(".$second.");\n";
		  #$isa_print_print.='%s%s(%s) ';
		}elsif($res[0]=~/^omem2$/){
			my $first;
			my $second;
			my $third;
			my $fourth;
			$first=$opnd_n+3;
			$second=$opnd_n;
			$third=$opnd_n+1;
			$fourth=$opnd_n+2;
			$isa_print_print.="Segment();\n";
			$isa_print_print.="Operand(".$first.");\n";
			$isa_print_print.="Operand(".$second.");\n";
			$isa_print_print.="Operand(".$third.");\n";
			$isa_print_print.="Operand(".$fourth.");\n";

		  #$isa_print_print.='%s%s(%s,%s,%s) ';
			$total_opnd+=4;
		}elsif($res[0]=~/^omem3$/){
			my $first;
			my $second;
			my $third;
			$first=$opnd_n+2;
			$second=$opnd_n;
			$third=$opnd_n+1;
			$isa_print_print.="Segment();\n";
			$isa_print_print.="Operand(".$first.");\n";
			$isa_print_print.="Operand(".$second.");\n";
			$isa_print_print.="Operand(".$third.");\n";
			#$isa_print_print.='%s%s(,%s,%s) ';
		}elsif($res[0]=~/^osimm8$/){
			$isa_print_print.="Result(0);\n";
		  #$isa_print_print.='%s';
		}elsif($res[0]=~/^o(x|y)mm/){
			$isa_print_print.="Result(0);\n";
			#$isa_print_print.='%s';
		}elsif($res[0]=~/^oint/){
			if(!($res[0]=~/rflag/)){
			 $isa_print_print.="Result(0);\n";
			  #$isa_print_print.='%s';
			}
		}
		else{
		  $isa_print_print.="TODO NYI isa_print_print=result".$res[0]."\n";
		}
	}

##end operand()
##end Result

	#print "TODO defined format here\n";
	$isa_print_print.="\tInstruction_Print_Group(".$_.",\n";
	foreach (@{$isa_operands{$_}}){
		$isa_print_print.="\t\t".$_->[0].",\n";
	}
	$isa_print_print.="\t\tTOP_UNDEFINED);\n\n\n";
}

print "\nisa_subset.cxx\n";
foreach (@tops){
	$isa_subset_print.="\t".$_.",\n";
}

my $print_load_only;
my $print_load_exe;
my $print_store_only;
my $print_move_prop;
my $print_fadd_prop;
my $print_fsub_prop;
my $print_fmul_prop;
my $print_flop_prop;
my $print_change_rflags;
my $print_xop_fma_vector;
my $print_madd_prop;
print "\nxop_fma_properties.cxx:\n";
foreach (@tops){
	$isa_properties_print.="\t".$_.",\n";
	if(type_operation($_) eq "load_only"){
	  $print_load_only.=$_.",\n";
	}
	if(type_operation($_) eq "load_exe"){
	  $print_load_exe.=$_.",\n";
	}
	if(type_operation($_) eq "store_only"){
	  $print_store_only.=$_.",\n";
	}
	if(type_operation($_) eq "move_prop"){
	  $print_move_prop.=$_.",\n";
	}
	if(type_operation($_) eq "fadd_prop"){
	  $print_fadd_prop.=$_.",\n";
	}
	if(type_operation($_) eq "fsub_prop"){
	  $print_fsub_prop.=$_.",\n";
	}
	if(type_operation($_) eq "fmul_prop"){
	  $print_fmul_prop.=$_.",\n";
	}
	if($_=~/f256/){
	  $print_xop_fma_vector.=$_.",\n";
	}
	if($_=~/rflag/){
		$print_change_rflags.=$_.",\n";
	}
	if($_=~/float/){
	  $print_flop_prop.="\t".$_.",\n";
	}
}

print "\nxop_fma_pack.cxx:\n";
foreach (@tops){
	$isa_pack_print.="\t".$_.", 0x000000ff,\n";
}


print "\nxop_fma_si.cxx:\n";
	$isa_si_print.="//TODO should apart these instructions accordingly!\n\n\n";
	$isa_si_print.="\tInstruction_Group(\""."xop_fma with memory"."\",\n";
foreach (@tops){
	#foreach (@{$isa_operands{$_}}){
	if(access_memory($_)){
		$isa_si_print.="\t\t".$_.",\n";
	}
		#}
}
	$isa_si_print.="\t\tTOP_UNDEFINED);\n";
	$isa_si_print.="Any_Operand_Access_Time(0);\n";
	$isa_si_print.="Any_Result_Available_Time(4);\n";
	$isa_si_print.="Resource_Requirement(res_issue, 0);\n";
	$isa_si_print.="Resource_Requirement(res_fstore, 0);\n";
	
$isa_si_print.="\tInstruction_Group(\""."xop_fma arth"."\",\n";
foreach(@tops){
	if(!access_memory($_)){
		$isa_si_print.="\t\t".$_.",\n";
	}
}
	$isa_si_print.="\t\tTOP_UNDEFINED);\n";
	$isa_si_print.="Any_Operand_Access_Time(0);\n";
	$isa_si_print.="Any_Result_Available_Time(2);\n";
	$isa_si_print.="Resource_Requirement(res_fadd, 0);\n";
	$isa_si_print.="Resource_Requirement(res_issue, 0);\n";

foreach (@tops){
	#print $_."\n";
  my @tmp=split("_",$_);
  $isa_cgemit_xop_fma_print.="OP_Name[".$_."] = \""."$tmp[1]"."\";\n";
}

  fprint("isa_properties_xop_fma_vector.cxx", $print_xop_fma_vector);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_properties_xop_fma_vector.cxx") if($print_xop_fma_vector);

if (0){
fprint("isa_xop_fma.cxx", $isa_isa_print);
fprint("isa_xop_fma_print.cxx",$isa_print_print);
fprint("isa_xop_fma_operands.cxx",$isa_operands_print);
fprint("isa_xop_fma_pack.cxx",$isa_pack_print);
fprint("isa_xop_fma_subset.cxx",$isa_subset_print);
fprint("xop_fma_si2.cxx", $isa_si_print);
fprint("cgemit_targ_xop_fma.cxx", $isa_cgemit_xop_fma_print);

copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma.cxx");
copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_print.cxx");
copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_operands.cxx");
copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_pack.cxx");
copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_subset.cxx");
copy_to('../src/common/targ_info/proc/x8664/',"xop_fma_si2.cxx");
copy_to('../src/be/cg/x8664/cgemit_targ_xop_fma.cxx',"cgemit_targ_xop_fma.cxx");
  if(0){
		#change ebo_special and isa_properties by hand can be more effective
    fprint("isa_xop_fma_properties_load_only.cxx", $print_load_only);
    fprint("isa_xop_fma_properties_load_exe.cxx", $print_load_exe);
    fprint("isa_xop_fma_properties_store_only.cxx", $print_store_only);
    fprint("isa_xop_fma_properties_move_prop.cxx", $print_move_prop);
    fprint("isa_xop_fma_properties_fadd_prop.cxx", $print_fadd_prop);
    fprint("isa_xop_fma_properties_fsub_prop.cxx", $print_fsub_prop);
    fprint("isa_xop_fma_properties_fmul_prop.cxx", $print_fmul_prop);
    fprint("isa_xop_fma_properties_flop_prop.cxx", $print_flop_prop);
    fprint("isa_xop_fma_properties_change_rflags.cxx", $print_change_rflags);
    fprint("isa_xop_fma_properties_xop_fma_vector.cxx", $print_xop_fma_vector);
    fprint("ebo_special_xop_fma.cxx", $ebo_print);
    fprint("isa_xop_fma_properties.cxx",$isa_properties_print);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties.cxx");
		copy_to('../src/be/cg/x8664/',"ebo_special_xop_fma.cxx") if($ebo_print);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_load_only.cxx") if($print_load_only);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_load_exe.cxx") if ($print_load_exe);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_store_only.cxx") if($print_store_only);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_move_prop.cxx") if($print_move_prop);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_fadd_prop.cxx") if($print_fadd_prop);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_fsub_prop.cxx") if($print_fsub_prop);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_fmul_prop.cxx") if($print_fmul_prop);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_flop_prop.cxx") if($print_flop_prop);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_change_rflags.cxx") if($print_change_rflags);
  copy_to('../src/common/targ_info/isa/x8664/',"isa_xop_fma_properties_xop_fma_vector.cxx") if($print_xop_fma_vector);
		}
}
#sub copy_to_property{
	#fprint("isa_avx_properties_load_only.cxx", $print_load_only);
	# fprint("isa_avx_properties_load_exe.cxx", $print_load_exe);
#  fprint("isa_avx_properties_store_only.cxx", $print_store_only);
#  fprint("isa_avx_properties_move_prop.cxx", $print_move_prop);
#  fprint("isa_avx_properties_fadd_prop.cxx", $print_fadd_prop);
#  fprint("isa_avx_properties_fsub_prop.cxx", $print_fsub_prop);
#  fprint("isa_avx_properties_fmul_prop.cxx", $print_fmul_prop);
#  fprint("isa_avx_properties_flop_prop.cxx", $print_flop_prop);
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_load_only.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_load_exe.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_store_only.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_move_prop.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_fadd_prop.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_fsub_prop.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"isa_avx_properties_fmul_prop.cxx");
#  copy_to('../src/common/targ_info/isa/x8664/',"sa_avx_properties_flop_prop.cxx");
#}

#copy_to_property();
