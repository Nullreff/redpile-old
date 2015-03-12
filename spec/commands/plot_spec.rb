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
end
