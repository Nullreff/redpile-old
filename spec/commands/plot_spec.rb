require 'spec_helper'
include Helpers

def empty_plot(axis1, axis2)
  graph = <<EOF
+--------------------------------------------#{axis1}
|                                            
|                                            
|                                            
|                                            
|                                            
|                                            
|                                            
|                                            
|                                            
|                                            
|                                            
#{axis2}
EOF
  graph.strip
end

def torch_xy_plot
  graph = <<EOF
+--------------------------------------------Y
| 15   0   0   0   0   0   0   0   0   0   0 
| 15   0   0   0   0   0   0   0   0   0   0 
| 14   0   0   0   0   0   0   0   0   0   0 
| 13   0   0   0   0   0   0   0   0   0   0 
| 12   0   0   0   0   0   0   0   0   0   0 
| 11   0   0   0   0   0   0   0   0   0   0 
| 10   0   0   0   0   0   0   0   0   0   0 
|  9   0   0   0   0   0   0   0   0   0   0 
|  8   0   0   0   0   0   0   0   0   0   0 
|  7   0   0   0   0   0   0   0   0   0   0 
|  6   0   0   0   0   0   0   0   0   0   0 
X
EOF
  graph.strip
end

def torch_xz_plot
  graph = <<EOF
+--------------------------------------------Z
| 15  15  14  13  12  11  10   9   8   7   6 
| 15  14  13  12  11  10   9   8   7   6   5 
| 14  13  12  11  10   9   8   7   6   5   4 
| 13  12  11  10   9   8   7   6   5   4   3 
| 12  11  10   9   8   7   6   5   4   3   2 
| 11  10   9   8   7   6   5   4   3   2   1 
| 10   9   8   7   6   5   4   3   2   1   0 
|  9   8   7   6   5   4   3   2   1   0   0 
|  8   7   6   5   4   3   2   1   0   0   0 
|  7   6   5   4   3   2   1   0   0   0   0 
|  6   5   4   3   2   1   0   0   0   0   0 
X
EOF
  graph.strip
end

def torch_yz_plot
  graph = <<EOF
+--------------------------------------------Z
| 15  15  14  13  12  11  10   9   8   7   6 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
|  0   0   0   0   0   0   0   0   0   0   0 
Y
EOF
  graph.strip
end

describe 'PLOT' do
  it 'draws an empty xy plot' do
    run('PLOT 0..10,0..10,0 power').should == empty_plot('Y', 'X')
  end

  it 'draws an empty xz plot' do
    run('PLOT 0..10,0,0..10 power').should == empty_plot('Z', 'X')
  end

  it 'draws an empty yz plot' do
    run('PLOT 0,0..10,0..10 power').should == empty_plot('Z', 'Y')
  end

  it 'draws a torch xy plot' do
    run(
      'node 0..10,0..10,0..10 wire',
      'node 0,0,0 torch direction:up',
      'tickq 2',
      'plot 0..10,0..10,0 power'
    ).should == torch_xy_plot
  end

  it 'draws a torch xz plot' do
    run(
      'node 0..10,0..10,0..10 wire',
      'node 0,0,0 torch direction:up',
      'tickq 2',
      'plot 0..10,0,0..10 power'
    ).should == torch_xz_plot
  end

  it 'draws a torch yz plot' do
    run(
      'node 0..10,0..10,0..10 wire',
      'node 0,0,0 torch direction:up',
      'tickq 2',
      'plot 0,0..10,0..10 power'
    ).should == torch_yz_plot
  end
end
