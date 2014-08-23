//--------------------------------------------------------------------------
// ComRing Interaction Example                                   by Bob Hood
//
// This LScript uses the LightWave ComRing system to interactively alter
// the appearance of a custom-object plug-in applied to one or more camera
// objects.
//
// Requires the ringtest.p C plug-in to be installed into Layout to function
// correctly.
//

@warnings
@script master
@version 2.7

@define RINGNAME "ringtest_channel"

camera_list;

camera_radii;
camera_target;
camera_active;

selected_camera;
target_object;
ctl;

@sequence { CAMERA_CTL, RADIUS_CTL, EFFECTOR_CTL, TARGETCAM_CTL, CAMACTIVE_CTL }

create
{
    selected_camera = nil;

    camera_list = nil;
    camera_radii = nil;

    target_object = nil;

    comringattach(RINGNAME,"comringevent");

    setdesc("ComRing Tester");
}

destroy
{
    comringdetach(RINGNAME);

    if(camera_active)
    {
        names = camera_active.keys();
        foreach(i,names)
        {
            if(camera_active[i])
            {
                SelectByName(i);
                RemoveServer("CustomObjHandler",1);
            }
        }
    }
}

comringevent: event, data
{
    // we'll get a message from the ringtest.p plug-in as
    // soon as we activate it on a camera.  the data transmitted
    // to us is just for testing purposes.

    if(event == 1)
    {
        (s1,s2,s3,i,d,f1,f2,f3,f4,f5) = comringdecode(@"s:100#3","i","d","f#5"@,data);

        // show the strings
        info("'" + s1 + "' '" + s2 + "' '" + s3 + "'");

        // the integer and double values
        info(i.asStr() + " " + d);

        // and the array of float values
        info(f1.asStr() + " " + f2 + " " + f3 + " " + f4 + " " + f5);
    }
}

flags
{
    return(SCENE);    // script will be cleared with scene
}

options
{
    if(reqisopen())
        reqend();
    else
    {
        reqbegin("ComRing Interaction Example");
        reqsize(350,120);

        ctl[EFFECTOR_CTL] = ctlallitems("Target",nil);
        ctlrefresh(ctl[EFFECTOR_CTL],"effector_change");

        ctl[CAMERA_CTL] = ctlcameraitems("Camera",nil);
        ctlrefresh(ctl[CAMERA_CTL],"camera_change");

        ctl[CAMACTIVE_CTL] = ctlcheckbox("Active",false);
        ctlrefresh(ctl[CAMACTIVE_CTL],"active_change");

        ctl[RADIUS_CTL] = ctlpercent("Radius",1.0);
        ctlrefresh(ctl[RADIUS_CTL],"radius_change");

        ctl[TARGETCAM_CTL] = ctlcheckbox("Target object",false);
        ctlrefresh(ctl[TARGETCAM_CTL],"target_change");

        ctlactive(ctl[CAMERA_CTL],"camera_active",ctl[CAMACTIVE_CTL]);

        ctl[RADIUS_CTL].active(false);
        ctl[TARGETCAM_CTL].active(false);

        control_positions();    // set the control positions based on panel dimensions

        reqredraw("redraw");

        reqopen();
    }
}

camera_active: val
{
    return(val != nil);
}

control_positions
{
    ctlposition(ctl[EFFECTOR_CTL],70,15);

    ctlposition(ctl[CAMERA_CTL],15,55);
    ctlposition(ctl[CAMACTIVE_CTL],220,55);
    ctlposition(ctl[RADIUS_CTL],18,85);
    ctlposition(ctl[TARGETCAM_CTL],158,85);
}

redraw
{
    drawborder(5,ctl[EFFECTOR_CTL].y - 5,330,30,true);
    drawborder(5,ctl[CAMERA_CTL].y - 5,330,60,true);
    drawborder(0,0,350,120,false);
}

camera_change: val
{
    selected_camera = val;

    if(val)
    {
        SelectItem(val.id);

        if(!camera_active || !camera_active[val.name])
        {
            ctl[CAMACTIVE_CTL].value = false;
            ctl[RADIUS_CTL].active(false);
            ctl[TARGETCAM_CTL].active(false);
        }
        else
        {
            if(!camera_radii || !camera_radii[val.name])
                camera_radii[val.name] = 1.0;

            if(!camera_radii || !camera_radii[val.name])
                camera_radii[val.name] = 1.0;

            if(!camera_target || !camera_target[val.name])
                camera_target[val.name] = false;

            ctl[CAMACTIVE_CTL].value = camera_active[val.name];

            if(camera_active[val.name])
            {
                ctl[RADIUS_CTL].active(true);
                ctl[TARGETCAM_CTL].active(true);

                ctl[RADIUS_CTL].value = camera_radii[val.name];
                ctl[TARGETCAM_CTL].value = camera_target[val.name];
            }
        }
    }
    else
    {
        ctl[RADIUS_CTL].active(false);
        ctl[TARGETCAM_CTL].active(false);
    }
}

radius_change: val
{
    if(!selected_camera) return;
    camera_radii[selected_camera.name] = val;

    // tell the ringtest.p plug-in to alter its radius

    data = comringencode(@"d"@,val * 1.0);
    comringmsg(RINGNAME,selected_camera.id,data);
}

target_change: val
{
    if(!selected_camera) return;
    camera_target[selected_camera.name] = val;

    if(target_object)
    {
        SelectByName(selected_camera.name);
        if(val)
            TargetItem(target_object.name);
        else
            TargetItem(0);
    }

    // send some sample data to the ringtest.p plug-in
    // each time the target is changed.  this is just
    // dummy data

    strings = @"Falicitations!","Salutations!","Greetings!"@;
    floats = @6.6666,7.7777,8.8888,9.9999,0.0000@;

    data = comringencode(@"s:100#3","i","d","f#5"@,strings,67832,34.8765,floats);
    comringmsg(RINGNAME,2,data);
}

active_change: val
{
    if(!selected_camera) return;
    camera_active[selected_camera.name] = val;

    ctl[RADIUS_CTL].active(val);
    ctl[TARGETCAM_CTL].active(val);

    // add/remove the RingTest plug-in

    if(val)
        ApplyServer("CustomObjHandler","ComRingTest");
    else
        RemoveServer("CustomObjHandler",1);
}

effector_change: val
{
    target_object = val;

    if(val)
    {
        // tell the ringtest.p plug-in it has a new
        // target object

        data = comringencode(@"i"@,val.id);
        comringmsg(RINGNAME,1,data);

        if(camera_target)
        {
            foreach(i,camera_target.keys())
            {
                if(camera_target[i])
                {
                    SelectByName(i);
                    TargetItem(target_object.name);
                }
            }

            RefreshNow();
        }
    }
    else
    {
        // tell the ringtest.p plug-in it has no
        // target object

        data = comringencode(@"i"@,0);
        comringmsg(RINGNAME,1,data);
    }
}

process: event, command
{
    // this Master script doesn't process any system commands.
    // it's simply here to support the Requester panel and the
    // ComRing interaction.
}
