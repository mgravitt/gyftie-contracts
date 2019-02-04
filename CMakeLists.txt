
cmake_minimum_required(VERSION 3.5)                                                                                                                                    
project(Gyftie VERSION 1.0.0)                                                                                                                                   
                                                                                                                                                                       
add_subdirectory (gyftietoken)                                                                                                                                            
add_subdirectory (genesisgen) 
add_subdirectory (secondgen)
add_subdirectory (token)
add_subdirectory (gftorderbook)
add_subdirectory (gftmultisig)