/*
Notes:
    Performance
        Analytic intersections are used instead of ray marching a SDF
        Normals are computed by sampling a SDF-version of the scene
        Many iterations will sample "Gradient" -> poor performance
    Scene
        Very high quality scene which is extremely artistic
    ReSTIR
        All meaningful attributes are stored together with reservoir positions to improve reprojection quality
    Reprojection
        Based on nearest neighbour gathering
            Search in a 3x3 pixel area inside the last frame screen to find good reservoirs
            To completely remove smearing/distortion a scattering approach is better
                A 3x3 search area is enough
                The main cost is increased noise, which can be reduced with an additional spatial pass
        No motion vectors
    Adaptivity
        Old rays are retraced every third frame to remove stale samples when lights/geometry have moved
    Spatial reservoirs
        Visibility is approximated with a screen space ray marcher
            It is bad because 2D samples are not uniformly distributed in screen space, just in world space
            It also assumes pixels have infinite thickness behind their depths
    Shadows are denoised using a two pass filter
    TAA
        Only reprojection and color clamping is used

Controls:
    WASD to move the camera
    Mouse to rotate the camera
    M/N to rotate the sun
*/

vec4 textureCube(vec2 UV) {
    //Samples the cubemap
    float Sign = -mod(floor(UV.y*I1024),2.)*2.+1.;
    vec3 D = vec3(vec2(UV.x,mod(UV.y,1024.))*I512-1.,Sign);
    if (UV.y>4096.) D = D.xzy;
    else if (UV.y>2048.) D = D.zxy;
    return texture(iChannel3,D);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    float YOffset = floor(fragCoord.x*I1024)*1024.+floor(fragCoord.y*I1024)*3072.;
    vec3 Color = textureCube(mod(fragCoord,1024.)+vec2(0.,YOffset)).xyz;
    fragColor=vec4(pow(1.-exp(-1.2*Color),vec3(0.45)),1.);
}

///// Buffer 1 //////

//Storage + Depth + Secondary rays

vec3 Sample_L(vec3 RayP, vec3 RayD, vec3 SunDir, vec2 Rand2, out float SampleD) {
    //Returns 1 or 1+2 bounced lighting
    vec3 L = vec3(0.);
    //First bounce
    SampleD = Trace(RayP,RayD,iTime);
    if (SampleD<=FAR) {
        //Geometry hit
        vec3 SampleP = RayP+RayD*SampleD;
        vec4 SampleSDF; vec3 SampleN = Gradient(SampleP,iTime,SampleSDF);
        if (SampleSDF.x>1.) {
            //Emissive
            L += EmissiveStrength*vec3(SampleSDF.x-1.,SampleSDF.yz);
        } else {
            //Diffuse
            vec3 Rand3 = clamp(ARand23(Rand2*9.234),vec3(0.0001,0.0001,0.),vec3(0.9999,0.9999,1.));
            vec3 RandDir = normalize(RandSampleCos(Rand3.xy)*TBN(SunDir)*SunCR+SunDir);
            if (dot(SampleN,RandDir)>0.) {
                if (Trace(SampleP+SampleN*0.001,RandDir,iTime)>FAR) L += SunLight*SampleSDF.xyz*max(0.,dot(SampleN,RandDir));
            }
            //Second bounce
            vec3 RayD2 = RandSample(Rand3.xy)*TBN(SampleN);
            float SampleD2 = Trace(SampleP+SampleN*0.001,RayD2,iTime);
            if (SampleD2<=FAR) {
                //Geometry hit
                vec3 SampleP2 = SampleP+SampleN*0.001+RayD2*SampleD2;
                vec4 SampleSDF2; vec3 SampleN2 = Gradient(SampleP2,iTime,SampleSDF2);
                if (SampleSDF2.x>1.) L += EmissiveStrength*SampleSDF.xyz*vec3(SampleSDF2.x-1.,SampleSDF2.yz);
                else {
                    //Diffuse
                    Rand3 = clamp(ARand23(Rand2*3.234),vec3(0.0001,0.0001,0.),vec3(0.9999,0.9999,1.));
                    RandDir = normalize(RandSampleCos(Rand3.xy)*TBN(SunDir)*SunCR+SunDir);
                    if (dot(SampleN2,RandDir)>0.) {
                        if (Trace(SampleP2+SampleN2*0.001,RandDir,iTime)>FAR)
                            L += SunLight*SampleSDF.xyz*SampleSDF2.xyz*max(0.,dot(SampleN2,RandDir));
                    }
                }
            } else {
                //Sky hit
                L += SampleSky(RayD2,iTime)*SampleSDF.xyz;
            }
        }
    } else {
        //Sky hit
        L += SampleSky(RayD,iTime);
    }
    return L;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec4 Output = texture(iChannel0,fragCoord.xy*IRES);
    if (iFrame==0) { //Initialization
        if (fragCoord.x<10. && fragCoord.y<1.) { //Store vars
            if (fragCoord.x<1.) Output = vec4(0.,0.,0.,0.); //Mouse
            else if (fragCoord.x<2.) Output = vec4(0.,-0.7,0.,0.); //Player Eye (Angles)
            else if (fragCoord.x<3.) Output = vec4(0.,0.,0.,1.); //Player Eye (Vector)
            else if (fragCoord.x<4.) Output = vec4(5.5,1.7,0.5,1.); //Player Pos
            else if (fragCoord.x<5.) Output = vec4(0.4,-2.15,0.,0.); //Sun angles
            else if (fragCoord.x<6.) Output = vec4(0.,0.,0.,0.); //Sun direction
        }
    } else { //Update
		if (fragCoord.x<16. && fragCoord.y<1.) { //Update vars
            if (fragCoord.x<1.) { //Mouse
                if (iMouse.z>0.) { //Börjat klicka
                    if (Output.w==0.) {
                    	Output.w = 1.;
                    	Output.xy = iMouse.zw;
                    }
                } else Output.w = 0.;
            } else if (fragCoord.x<2.) { //Player Eye (Angles)
                vec4 LMouse = texture(iChannel0,vec2(0.5,0.5)*IRES);
                if (LMouse.w==0.)  Output.zw = Output.xy;
                if (LMouse.w==1.) {
                	//Y led
                	Output.x = Output.z+(iMouse.y-LMouse.y)*0.01;
                	Output.x = clamp(Output.x,-2.8*0.5,2.8*0.5);
                	//X led
                	Output.y = Output.w-(iMouse.x-LMouse.x)*0.02;
               		Output.y = mod(Output.y,3.1415926*2.);
                }
            } else if (fragCoord.x<3.) { //Player Eye (Vector)
                vec3 Angles = texture(iChannel0,vec2(1.5,0.5)*IRES).xyz;
                Output.xyz = normalize(vec3(cos(Angles.x)*sin(Angles.y),
                  			   			sin(Angles.x),
                  			   			cos(Angles.x)*cos(Angles.y)));
            } else if (fragCoord.x<4.) { //Player Pos
                float Speed = iTimeDelta;
                	if (texelFetch(iChannel1,ivec2(32,0),0).x>0.) Speed = 5.*iTimeDelta;
                vec3 Eye = texture(iChannel0,vec2(2.5,0.5)*IRES).xyz;
                if (texelFetch(iChannel1,ivec2(87,0),0).x>0.) Output.xyz += Eye*Speed; //W
                if (texelFetch(iChannel1,ivec2(83,0),0).x>0.) Output.xyz -= Eye*Speed; //S
                vec3 Tan = normalize(cross(vec3(Eye.x,0.,Eye.z),vec3(0.,1.,0.)));
                if (texelFetch(iChannel1,ivec2(65,0),0).x>0.) Output.xyz -= Tan*Speed; //A
                if (texelFetch(iChannel1,ivec2(68,0),0).x>0.) Output.xyz += Tan*Speed; //D
            } else if (fragCoord.x<5.) { //Sun angle
                if (texelFetch(iChannel1,ivec2(77,0),0).x>0.) Output.y += iTimeDelta*1.5;
                if (texelFetch(iChannel1,ivec2(78,0),0).x>0.) Output.y -= iTimeDelta*1.5;
                Output.z = Output.y; //Sunangle last frame
            } else if (fragCoord.x<6.) { //Sun direction
                vec2 Angles = texture(iChannel0,vec2(4.5,0.5)*IRES).xy;
                Output = vec4(normalize(vec3(cos(Angles.y)*cos(Angles.x)
                	,sin(Angles.x),sin(Angles.y)*cos(Angles.x))),1.);
            } else if (fragCoord.x<7.) { //Last frame dir
                Output = texture(iChannel0,vec2(2.5,0.5)*IRES);
            } else if (fragCoord.x<8.) { //Last frame position
                Output = texture(iChannel0,vec2(3.5,0.5)*IRES);
            } else if (fragCoord.x<9.) { //Last frame SunDir
                Output = texture(iChannel0,vec2(5.5,0.5)*IRES);
            } else if (fragCoord.x<10.) { //Last last frame dir
                Output = texture(iChannel0,vec2(6.5,0.5)*IRES);
            } else if (fragCoord.x<11.) { //Last last frame position
                Output = texture(iChannel0,vec2(7.5,0.5)*IRES);
            }
        } else if (DFBox(fragCoord-vec2(1.),RES-2.)<0.) {
            vec2 SSOffset = SSOffsets[iFrame%16];
            float CurrentFrame = float(iFrame);
            vec2 UV = fragCoord+SSOffset;
            vec3 SunDir = texture(iChannel0,vec2(5.5,0.5)*IRES).xyz;
            vec3 Pos = texture(iChannel0,vec2(3.5,0.5)*IRES).xyz;
            vec3 Eye = texture(iChannel0,vec2(2.5,0.5)*IRES).xyz;
            vec3 Tan; vec3 Bit = TBN(Eye,Tan);
            vec3 LPos = texture(iChannel0,vec2(7.5,0.5)*IRES).xyz;
            vec3 LEye = texture(iChannel0,vec2(6.5,0.5)*IRES).xyz;
            vec3 LTan; vec3 LBit = TBN(LEye,LTan);
            mat3 LEyeMat = TBN(LEye);
            vec3 Dir = normalize(vec3((UV*IRES*2.-1.)*CFOV*ASPECT,1.)*TBN(Eye));
            float PixelD = Trace(Pos,Dir,iTime);
            if (PixelD<=FAR) {
                //Geometry
                vec3 PPos = Pos+Dir*PixelD;
                vec4 PixelSDF; vec3 Normal = Gradient(PPos,iTime,PixelSDF);
                
                
                
                
                //
                //Diffuse ray
                //
                vec3 RandV = ARand23(fragCoord*IRES*(1.+mod(CurrentFrame*7.253,9.234)));
                RandV.xy = (floor(RandV.xy*2048.)+0.5)*I2048;
                vec2 ShadowRandV = RandV.xy;
                float DiffuseDist;
                vec3 DPPos = PPos+Normal*0.001;
                vec3 DNormal = Normal;
                if (iFrame%3==0) {
                    //Re-trace ray
                    vec3 CVPos = PPos-LPos;
                    vec3 LVPos = vec3(dot(CVPos,LTan),dot(CVPos,LBit),dot(CVPos,LEye));
                    vec2 LuvCenter = floor(((LVPos.xy/LVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES);
                    float SmallestDistance = 1000.; vec2 LResUV = vec2(-1.);
                    vec3 RayStartPos = vec3(-1.); vec3 RayNormal = vec3(-1.);
                    for (float x = -1.; x<1.5; x++) {
                        for (float y = -1.; y<1.5; y++) {
                            vec4 LRSample = texture(iChannel3,(LuvCenter+0.5+vec2(x,y))*IRES);
                            vec2 LRluv = LuvCenter+vec2(x,y)+FloatToVec2(LRSample.y);
                            vec3 LRDir = normalize(vec3((LRluv*IRES*2.-1.)*CFOV*ASPECT,1.)*LEyeMat);
                            vec3 LRPPos = LPos+LRDir*LRSample.x;
                            vec4 LRSDF; vec3 LRNormal = Gradient(LRPPos,iTime,LRSDF);
                            //Reprojection on current screen space
                            vec3 RVPos = LRPPos-Pos;
                            RVPos = vec3(dot(RVPos,Tan),dot(RVPos,Bit),dot(RVPos,Eye));
                            vec2 LRUV = ((RVPos.xy/RVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                            float LRUVDist = length(fragCoord-LRUV);
                            if (LRSample.x<FAR && DFBox(LRUV-1.,RES-2.)<0. && abs(dot(Normal,LRPPos-PPos))<0.05
                                && dot(Normal,LRNormal)>0.9 && LRUVDist<SmallestDistance) {
                                SmallestDistance = LRUVDist;
                                LResUV = LRluv;
                                RayStartPos = LRPPos+LRNormal*0.001;
                                RayNormal = LRNormal;
                            }
                        }
                    }
                    if (DFBox(LResUV-1.,RES-2.)<0.) {
                        //Reprojection coordinate is outside of the last frame screen
                        DPPos = RayStartPos;
                        RandV.xy = FloatToVec2(texture(iChannel2,LResUV*IRES).y);
                        DNormal = RayNormal;
                    }
                }
                
                
                vec3 RandDir = RandSample(RandV.xy)*TBN(DNormal);
                vec3 DiffuseLight = Sample_L(DPPos,RandDir,SunDir,RandV.xy,DiffuseDist);
                
                
                
                
                //
                //Shadow ray
                //
                RandDir = normalize(RandSampleCos(ShadowRandV)*TBN(SunDir)*SunCR+SunDir);
                float ShadowDist = -1.;
                if (dot(Normal,RandDir)>0.) ShadowDist = Trace(DPPos,RandDir,iTime);
                //Output
                Output = vec4(PixelD,Vec3ToFloat(DiffuseLight*ILightCoeff),DiffuseDist,ShadowDist);
            } else {
                //Sky
                Output = vec4(FAR+10.,-1.,-1.,-1.);
            }
        }
    }
    fragColor = Output;
}

//// Buffer 2 ////

//Reprojection of the reservoir positions + Shadow denoising

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec4 Output = vec4(0.);
    if (DFBox(fragCoord-vec2(1.),RES-2.)<0.) {
        vec4 RefShad = texture(iChannel0,fragCoord*IRES);
        if (RefShad.x<=FAR) {
            //
            //Normal
            //
            vec2 SSOffset = SSOffsets[iFrame%16];
            float CurrentFrame = float(iFrame);
            vec2 UV = fragCoord+SSOffset;
            vec3 SunDir = texture(iChannel0,vec2(8.5,0.5)*IRES).xyz;
            vec3 Pos = texture(iChannel0,vec2(7.5,0.5)*IRES).xyz;
            vec3 Eye = texture(iChannel0,vec2(6.5,0.5)*IRES).xyz;
            vec3 Tan; vec3 Bit = TBN(Eye,Tan);
            mat3 EyeMat = TBN(Eye);
            vec3 LPos = texture(iChannel0,vec2(10.5,0.5)*IRES).xyz;
            vec3 LEye = texture(iChannel0,vec2(9.5,0.5)*IRES).xyz;
            vec3 LTan; vec3 LBit = TBN(LEye,LTan);
            mat3 LEyeMat = TBN(LEye);
            vec3 Dir = normalize(vec3((UV*IRES*2.-1.)*CFOV*ASPECT,1.)*EyeMat);
            vec3 PPos = Pos+Dir*RefShad.x;
            vec4 PSDF; vec3 Normal = Gradient(PPos,iTime,PSDF);
            Output.w = Vec3ToFloat(Normal*0.5+0.5);
            
            
            
            
            //
            //Reservoir position
            //
            vec3 CVPos = PPos-LPos;
            vec3 LVPos = vec3(dot(CVPos,LTan),dot(CVPos,LBit),dot(CVPos,LEye));
            vec2 Luv = ((LVPos.xy/LVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
            vec2 LuvCenter = floor(Luv);
            float SmallestDistance = 1000.; float WSDist; vec2 ResUV;
            for (float x = -1.; x<1.5; x++) {
                for (float y = -1.; y<1.5; y++) {
                    vec4 LRSample = texture(iChannel3,(LuvCenter+0.5+vec2(x,y))*IRES);
                    vec2 LRluv = LuvCenter+vec2(x,y)+FloatToVec2(LRSample.y);
                    vec3 LRDir = normalize(vec3((LRluv*IRES*2.-1.)*CFOV*ASPECT,1.)*LEyeMat);
                    vec3 LRPPos = LPos+LRDir*LRSample.x;
                    vec4 LRSDF; vec3 LRNormal = Gradient(LRPPos,iTime,LRSDF);
                    //Reprojection on current screen space
                    vec3 RVPos = LRPPos-Pos;
                    RVPos = vec3(dot(RVPos,Tan),dot(RVPos,Bit),dot(RVPos,Eye));
                    vec2 LRUV = ((RVPos.xy/RVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                    float LRUVDist = length(UV*0.+fragCoord-LRUV);
                    if (LRSample.x<FAR && DFBox(LRUV-1.,RES-2.)<0. && abs(dot(Normal,LRPPos-PPos))<0.05
                        && dot(Normal,LRNormal)>0.9 && LRUVDist<SmallestDistance) {
                        SmallestDistance = LRUVDist;
                        WSDist = length(LRPPos-Pos);
                        ResUV = LRUV;
                    }
                }
            }
            vec2 UVResidual = ResUV-floor(fragCoord);
            if (DFBox(UVResidual,vec2(1.))<=0.) {
                //Inside current pixel -> keep position
                Output.xy = vec2(WSDist,Vec2ToFloat(UVResidual));
            } else {
                //Outside current pixel -> new positions
                Output.xy = vec2(RefShad.x,Vec2ToFloat(SSOffset+0.5));
            }
            
            
            

            //
            //Shadow denoising pass 1
            //
            if (RefShad.w>-0.5) {
                //Surface is pointing towards SunDir
                vec3 CVPos0 = vec3(RefShad.w*SunCR,0.,RefShad.x+RefShad.w)*TBN(Dir);
                vec3 CVPos1 = vec3(-RefShad.w*SunCR,0.,RefShad.x+RefShad.w)*TBN(Dir);
                vec3 LVPos0 = vec3(dot(CVPos0,Tan),dot(CVPos0,Bit),dot(CVPos0,Eye));
                vec3 LVPos1 = vec3(dot(CVPos1,Tan),dot(CVPos1,Bit),dot(CVPos1,Eye));
                vec2 Luv0 = ((LVPos0.xy/LVPos0.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                vec2 Luv1 = ((LVPos1.xy/LVPos1.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                float MaxRadius = min(32.,length(Luv0-Luv1)*0.5); vec4 ssdf;
                vec2 CShadow = vec2(float(RefShad.w>FAR)*2.,2.);
                for (float x=-2.; x<2.5; x+=1.) {
                    for (float y=-2.; y<2.5; y+=1.) {
                        if (x==0. && y==0.) continue;
                        vec2 Offset2 = normalize(vec2(x,y))*max(abs(x),abs(y))*(MaxRadius*0.5);
                        vec2 SUV = floor(fragCoord+Offset2)+0.5;
                        vec4 SRefShad = texture(iChannel0,SUV*IRES);
                        float SDistance = SRefShad.x;
                        vec3 SDir = normalize(vec3(((SUV+SSOffset)*IRES*2.-1.)*(ASPECT*CFOV),1.)*EyeMat);
                        if (DFBox(SUV-1.,RES-2.)>0. || SDistance>FAR || SRefShad.w<-0.5 ||
                            abs(dot(Pos+SDir*SDistance-PPos,Normal))>0.05 ||
                            dot(Gradient(Pos+SDir*SDistance,iTime,ssdf),Normal)<0.9) continue;
                        vec2 SRand = ARand23(SUV*IRES*(1.+mod(CurrentFrame*7.253,9.234))).xy;
                        vec3 HitP = Pos+SDir*SDistance+normalize(RandSampleCos(SRand.xy)*TBN(SunDir)*SunCR+SunDir)*SRefShad.w;
                        if (dot(HitP-PPos,Normal)<=0.) continue;
                        if (sqrt(1./dot(normalize(HitP-PPos),SunDir)-1.)<=SunCR) {
                            CShadow += vec2(float(SRefShad.w>FAR),1.);
                        }
                    }
                }
                Output.z = CShadow.x/CShadow.y;
            } else Output.z = -1.;
        } else Output = vec4(FAR+10.,-1.,-1.,-1.);
    }
    //Output
    fragColor = Output;
}


/// buffer 3 ///

//Temporal ReSTIR

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec4 Output = vec4(0.);
    //Diffuse temporal ReSTIR
    if (DFBox(fragCoord-vec2(1.),RES-2.)<0.) {
        float CurrentFrame = float(iFrame);
        vec2 SSOffset = SSOffsets[iFrame%16];
        vec2 UV = fragCoord+SSOffset;
        vec3 SunDir = texture(iChannel0,vec2(8.5,0.5)*IRES).xyz;
        vec3 Pos = texture(iChannel0,vec2(7.5,0.5)*IRES).xyz;
        vec3 Eye = texture(iChannel0,vec2(6.5,0.5)*IRES).xyz;
        vec3 Tan; vec3 Bit = TBN(Eye,Tan);
        mat3 EyeMat = TBN(Eye);
        vec3 LPos = texture(iChannel0,vec2(10.5,0.5)*IRES).xyz;
        vec3 LEye = texture(iChannel0,vec2(9.5,0.5)*IRES).xyz;
        vec3 LTan; vec3 LBit = TBN(LEye,LTan);
        mat3 LEyeMat = TBN(LEye);
        vec3 Dir = normalize(vec3((UV*IRES*2.-1.)*CFOV*ASPECT,1.)*EyeMat);
        vec4 DiffShad = texture(iChannel0,UV*IRES);
        if (DiffShad.x<=FAR) {
            //Geometry pixel
            vec3 PPos = Pos+Dir*DiffShad.x;
            vec4 CSDF; vec3 Normal = Gradient(PPos,iTime,CSDF);
            vec3 L = vec3(0.);
            //Reprojection
            vec3 CVPos = PPos-LPos;
            vec3 LVPos = vec3(dot(CVPos,LTan),dot(CVPos,LBit),dot(CVPos,LEye));
            vec2 Luv = ((LVPos.xy/LVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
            vec2 LuvCenter = floor(Luv);
            //Find reprojection pixel
            float SmallestDistance = 1000.; vec2 LResUV,ResUV;
            for (float x = -1.; x<1.5; x++) {
                for (float y = -1.; y<1.5; y++) {
                    vec4 LRSample = texture(iChannel3,(LuvCenter+0.5+vec2(x,y))*IRES);
                    vec2 LRluv = LuvCenter+vec2(x,y)+FloatToVec2(LRSample.y);
                    vec3 LRDir = normalize(vec3((LRluv*IRES*2.-1.)*CFOV*ASPECT,1.)*LEyeMat);
                    vec3 LRPPos = LPos+LRDir*LRSample.x;
                    vec4 LRSDF; vec3 LRNormal = Gradient(LRPPos,iTime-iTimeDelta,LRSDF);
                    //Reprojection on current screen space
                    vec3 RVPos = LRPPos-Pos;
                    RVPos = vec3(dot(RVPos,Tan),dot(RVPos,Bit),dot(RVPos,Eye));
                    vec2 LRUV = ((RVPos.xy/RVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                    float LRUVDist = length(UV*0.+fragCoord-LRUV);
                    if (LRSample.x<FAR && DFBox(LRUV-1.,RES-2.)<0. && abs(dot(Normal,LRPPos-PPos))<0.05
                        && dot(Normal,LRNormal)>0.9 && LRUVDist<SmallestDistance) {
                        SmallestDistance = LRUVDist;
                        ResUV = LRUV;
                        LResUV = LRluv;
                    }
                }
            }
            vec2 UVResidual = ResUV-floor(fragCoord);
            if (SmallestDistance>900.) {// || DFBox(UVResidual,vec2(1.))>0.) {
                //No valid reprojection on the last frame -> new pixel
                vec3 Rand3 = ARand23(fragCoord*IRES*(1.+mod(CurrentFrame*7.253,9.234)));
                Rand3.xy = (floor(Rand3.xy*2048.)+0.5)*I2048;
                Output = vec4(DiffShad.y,Vec2ToFloat(Rand3.xy),DiffShad.z,Vec2ToFloatWM(vec2(1.)));
            } else {
                //Old pixel
                vec4 LR = texture(iChannel2,LResUV*IRES);
                vec2 LRWM = FloatToVec2WM(LR.w);
                float M = LRWM.y;
                vec2 Rand2 = FloatToVec2(LR.y);
                vec3 RLight = FloatToVec3(LR.x)*LightCoeff;
                float RDist = LR.z;
                float w = max(0.,dot(RLight,vec3(0.3333)))*M*LRWM.x;
                float W = w/max(0.0001,M*dot(RLight,vec3(0.3333))); //Update W
                if (iFrame%3==0) {
                    //Sample validation
                    L = FloatToVec3(DiffShad.y)*LightCoeff;
                    if (length(RLight-L)>0.1) {
                        //Invalid sample
                        Output = vec4(Vec3ToFloat(L*ILightCoeff),LR.y,DiffShad.z,LR.w);
                    } else {
                        //Valid sample
                        Output = LR;
                    }
                } else {
                    //Temporal ReSTIR
                    vec3 Rand3 = ARand23(fragCoord*IRES*(1.+mod(CurrentFrame*7.253,9.234)));
                    Rand3.xy = (floor(Rand3.xy*2048.)+0.5)*I2048;
                    vec3 SRD = RandSample(Rand3.xy)*TBN(Normal);
                    float SampleD = DiffShad.z;
                    L = FloatToVec3(DiffShad.y)*LightCoeff;
                    float wnew = max(0.,dot(L,vec3(0.3333)))*dot(SRD,Normal); //Target pdf
                    M = min(M,M_CLAMP_T-1.); //Clamping
                    w = max(0.,dot(RLight,vec3(0.3333)))*RandSample(Rand2.xy).z*M*W+wnew; //R.w += w
                    if (Rand3.z<wnew/max(0.0001,w)) {
                        //New sample
                        RLight = L;
                        Rand2 = Rand3.xy;
                        RDist = SampleD;
                    }
                    M += 1.; //R.M += 1
                    float p_hat = max(0.,dot(RLight,vec3(0.3333)))*RandSample(Rand2.xy).z; //p hat
                    W = w/max(0.0001,M*p_hat); //Update W
                    //Output
                    Output = vec4(Vec3ToFloat(RLight*ILightCoeff),
                                  Vec2ToFloat(Rand2),
                                  RDist,
                                  Vec2ToFloatWM(vec2(W,M)));
                }
            }
        } else {
            //Sky pixel
            Output = vec4(0.,0.,0.,-1.);
        }
    }
    fragColor = Output;
}


/// buffer 4 ////


//Spatial ReSTIR + Shadow denoising + Copy reservoir positions

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec4 Output = vec4(0.);
    if (DFBox(fragCoord-vec2(1.),RES-2.)<0.) {
        //
        //Copy positions
        //
        Output.xy = texture(iChannel1,fragCoord*IRES).xy;
        float CurrentFrame = float(iFrame);
        vec2 SSOffset = SSOffsets[iFrame%16];
        vec2 UV = fragCoord+SSOffset;
        vec3 SunDir = texture(iChannel0,vec2(8.5,0.5)*IRES).xyz;
        vec3 Pos = texture(iChannel0,vec2(7.5,0.5)*IRES).xyz;
        vec3 Eye = texture(iChannel0,vec2(6.5,0.5)*IRES).xyz;
        vec3 Tan; vec3 Bit = TBN(Eye,Tan);
        mat3 EyeMat = TBN(Eye);
        vec3 LPos = texture(iChannel0,vec2(10.5,0.5)*IRES).xyz;
        vec3 LEye = texture(iChannel0,vec2(9.5,0.5)*IRES).xyz;
        vec3 LTan; vec3 LBit = TBN(LEye,LTan);
        mat3 LEyeMat = TBN(LEye);
        vec3 Dir = normalize(vec3((UV*IRES*2.-1.)*CFOV*ASPECT,1.)*EyeMat);
        vec4 DiffShad = texture(iChannel0,UV*IRES);
        if (DiffShad.x<=FAR) {
            //
            //ReSTIR spatial
            //
            vec3 PPos = Pos+Dir*DiffShad.x;
            vec4 CSDF; vec3 Normal = Gradient(PPos,iTime,CSDF);
            vec4 CR = texture(iChannel2,fragCoord*IRES);
            vec2 CRWM = FloatToVec2WM(CR.w);
            float W = CRWM.x;
            float M = CRWM.y;
            vec3 CRLight = FloatToVec3(CR.x)*LightCoeff;
            float CRRandFloat = CR.y;
            float w = max(0.,dot(CRLight,vec3(0.3333)))*M*W*RandSample(FloatToVec2(CRRandFloat)).z;
            vec3 Rand3 = ARand23(fragCoord*(1.+mod(iTime*18.327,13.9347)));
            int NSamples = 9;
            float SpatialRadius = 1.+Rand3.x //*(RES.x*0.1)
                                  *clamp((2./DiffShad.x)*RES.x*0.1,1.,RES.x*0.1);
            float AngleDelta = 6.28318530718/float(NSamples);
            float CAngle = Rand3.y*AngleDelta;
            float Jacobian,wnew,np_hat; vec2 SUV; vec3 SDir,SPos,SNormal,SRayHit,HitNormal; vec4 SDiffShad,SR,SSDF;
            for (int s=0; s<NSamples; s++) {
                //For all spatial reservoirs
                CAngle += AngleDelta;
                SUV = floor(fragCoord+vec2(sin(CAngle),cos(CAngle))*SpatialRadius)+0.5;
                if (DFBox(SUV-1.,RES-2.)>=0.) continue;
                SDiffShad = texture(iChannel0,SUV*IRES);
                if (SDiffShad.x>FAR) continue; //Sky pixel test
                SDir = normalize(vec3(((SUV-0.5+FloatToVec2(texture(iChannel1,SUV*IRES).y))*IRES*2.-1.)*CFOV*ASPECT,1.)*TBN(Eye));;
                SPos = Pos+SDir*SDiffShad.x;
                SNormal = Gradient(SPos,iTime,SSDF);
                if (!(dot(SNormal,Normal)>0.9 && abs(dot(SPos-PPos,Normal))<0.05)) continue; //Geometric similarity test
                SR = texture(iChannel2,SUV*IRES);
                vec3 SRandDir = (RandSample(FloatToVec2(SR.y))*TBN(SNormal));
                SRayHit = SPos+SR.z*SRandDir;
                HitNormal = Gradient(SRayHit,iTime,SSDF);
                if ((SR.z<FAR && dot(PPos-SRayHit,HitNormal)<=0.) || dot(SRayHit-PPos,Normal)<=0.) continue; //Hemisphere test
                
                //Exact visibility
                    //if (abs(Trace(PPos+Normal*0.001,normalize(SRayHit-PPos),iTime)-length(SRayHit-PPos))>0.05) continue;
                //Screen space ray tracing to approximate visibility
                    bool Visible = true;
                    float SSNSamples = 10.;
                    vec3 SSRayDir = normalize(SRayHit-PPos);
                    for (float di=ARand21(SUV)*0.8+0.1; di<SSNSamples; di++) {
                        //For each sample
                        vec3 SSP = PPos+SSRayDir*(di*0.1);
                        vec3 SSVPos = SSP-Pos;
                        SSVPos = vec3(dot(SSVPos,Tan),dot(SSVPos,Bit),dot(SSVPos,Eye));
                        vec2 SSUV = ((SSVPos.xy/SSVPos.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                        if (DFBox(SSUV-1.,RES-2.)>0.) break;
                        float SSDepth = texture(iChannel0,SSUV*IRES).x;
                        if (SSDepth+0.02<length(SSVPos)) { Visible = false; break; }
                    }
                    if (!Visible) continue;
                
                //Read reservoir
                vec2 SWM = FloatToVec2WM(SR.w);
                vec3 SRLight = FloatToVec3(SR.x)*LightCoeff;
                //Jacobian
                Jacobian = abs(dot(PPos-SRayHit,HitNormal)*pow(SR.z,3.))/
                           max(0.0001,abs(dot(SPos-SRayHit,HitNormal)*pow(length(PPos-SRayHit),3.)));
                if (SR.z>FAR) Jacobian = 1.; //Sky sample
                np_hat = max(0.,dot(SRLight,vec3(0.3333)));
                wnew = np_hat*SWM.y*SWM.x*RandSample(FloatToVec2(SR.y)).z*max(0.0001,Jacobian);
                w += wnew;
                M += SWM.y;
                float RandV = ARand21(SUV+mod(float(iFrame+s),2048.)*vec2(3.683,4.887));
                if (RandV<wnew/max(0.0001,w)) {
                    CRLight = SRLight;
                    CRRandFloat = SR.y;
                }
            }
            //Bias correction
            float bias_p_hat = max(0.,dot(CRLight,vec3(0.3333)))*RandSample(FloatToVec2(CRRandFloat)).z;
            W = w/max(0.0001,M*bias_p_hat);
            vec3 IndirectDiffuse = CRLight*W*RandSample(FloatToVec2(CRRandFloat)).z;




            //
            //Shadow denoising pass 2
            //
            vec4 RefShad = texture(iChannel0,fragCoord*IRES);
            vec2 CShadow = vec2(0.);
            if (RefShad.w>-0.5) {
                //Surface is pointing towards SunDir
                vec3 CVPos0 = vec3(RefShad.w*SunCR,0.,RefShad.x+RefShad.w)*TBN(Dir);
                vec3 CVPos1 = vec3(-RefShad.w*SunCR,0.,RefShad.x+RefShad.w)*TBN(Dir);
                vec3 LVPos0 = vec3(dot(CVPos0,Tan),dot(CVPos0,Bit),dot(CVPos0,Eye));
                vec3 LVPos1 = vec3(dot(CVPos1,Tan),dot(CVPos1,Bit),dot(CVPos1,Eye));
                vec2 Luv0 = ((LVPos0.xy/LVPos0.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                vec2 Luv1 = ((LVPos1.xy/LVPos1.z)*0.5/(ASPECT*CFOV)+0.5)*RES;
                float MaxRadius = min(8.,length(Luv0-Luv1)*0.5); vec4 ssdf;
                CShadow = vec2(texture(iChannel1,fragCoord*IRES).z*2.,2.);
                for (float x=-2.; x<2.5; x+=1.) {
                    for (float y=-2.; y<2.5; y+=1.) {
                        if (x==0. && y==0.) continue;
                        vec2 Offset2 = normalize(vec2(x,y))*max(abs(x),abs(y))*(MaxRadius*0.5);
                        vec2 SUV = floor(fragCoord+Offset2)+0.5;
                        vec4 SRefShad = texture(iChannel0,SUV*IRES);
                        float SDistance = SRefShad.x;
                        vec3 SDir = normalize(vec3(((SUV+SSOffset)*IRES*2.-1.)*(ASPECT*CFOV),1.)*EyeMat);
                        if (DFBox(SUV-1.,RES-2.)>0. || SDistance>FAR || SRefShad.w<-0.5 ||
                            abs(dot(Pos+SDir*SDistance-PPos,Normal))>0.05 ||
                            dot(Gradient(Pos+SDir*SDistance,iTime,ssdf),Normal)<0.9) continue;
                        vec2 SRand = ARand23(SUV*IRES*(1.+mod(CurrentFrame*7.253,9.234))).xy;
                        vec3 HitP = Pos+SDir*SDistance+normalize(RandSampleCos(SRand.xy)*TBN(SunDir)*SunCR+SunDir)*SRefShad.w;
                        if (dot(HitP-PPos,Normal)<=0.) continue;
                        if (sqrt(1./dot(normalize(HitP-PPos),SunDir)-1.)<=SunCR) {
                            CShadow += vec2(texture(iChannel1,SUV*IRES).z,1.);
                        }
                    }
                }
                CShadow.x = CShadow.x/CShadow.y;
            }




            //
            //Composition
            //
            vec3 FinalColor = IndirectDiffuse+CShadow.x*SunLight*max(0.,dot(Normal,SunDir));
            FinalColor *= CSDF.xyz;
            Output.zw = vec2(Vec2ToFloat(FinalColor.xy*0.2),FinalColor.z);
        } else {
            //Sky
            vec3 FinalColor = SampleSky(Dir,iTime);
            Output.zw = vec2(Vec2ToFloat(FinalColor.xy*0.2),FinalColor.z);
        }
    }
    fragColor = Output;
}

