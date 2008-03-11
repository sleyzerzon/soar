using System;
using System.Collections.Generic;
using System.Text;

using sml;
using System.Threading;
using System.ComponentModel;
using System.Diagnostics;

using bumper = Microsoft.Robotics.Services.ContactSensor.Proxy;
using drive = Microsoft.Robotics.Services.Drive.Proxy;
using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;

namespace Robotics.SoarMSR
{
    class Soar
    {
        // Logging
        public delegate void LogHandler(string message);
        public event LogHandler Log;
        protected void OnLog(string message)
        {
            if (Log != null)
            {
                Log(message);
            }
        }

        // Output
        drive.DriveOperations _drivePort;

        // Soar variables
        private sml.Kernel _kernel;
        private sml.Agent _agent;
        private sml.Kernel.UpdateEventCallback _updateCall;

        // Input link
        private StringElement _overrideActiveWME;
        private FloatElement _overrideLeftWME;
        private FloatElement _overrideRightWME;
        private StringElement _overrideStopWME;
        private StringElement _frontBumperWasPressedWME;
        private StringElement _frontBumperPressedWME;
        private FloatElement _frontBumperTimeWME;
        private StringElement _rearBumperWasPressedWME;
        private StringElement _rearBumperPressedWME;
        private FloatElement _rearBumperTimeWME;
        private FloatElement _timeWME;
        private FloatElement _randomWME;

        // Input link support
        private Random _random;

        // State
        private DateTime _simulationStart;
        private bool _running;
        private bool _stop;
        public BumperState Bumper = new BumperState();
        public OverrideState Override = new OverrideState();

        public void InitializeSoar(SoarMSRState initialState, drive.DriveOperations drivePort)
        {
            if (_kernel != null)
            {
                throw new Exception("Soar: Already initialized");
            }

            _kernel = sml.Kernel.CreateKernelInNewThread("SoarKernelSML");
            if (_kernel.HadError())
            {
                _kernel = null;
                throw new Exception("Soar: Error initializing kernel: " + _kernel.GetLastErrorDescription());
            }

            _running = false;
            _stop = false;

            _agent = _kernel.CreateAgent(initialState.AgentName);

            // We test the kernel for an error after creating an agent as the agent
            // object may not be properly constructed if the create call failed so
            // we store errors in the kernel in this case.  Once this create is done we can work directly with the agent.
            if (_kernel.HadError())
                throw new Exception("Soar: Error creating agent: " + _kernel.GetLastErrorDescription());

            _kernel.SetAutoCommit(false);
            _agent.SetBlinkIfNoChange(false);

            bool result = _agent.LoadProductions(initialState.Productions);
            if (!result)
            {
                throw new Exception("Soar: Error loading productions " + initialState.Productions
                    + " (current working directory: " + _agent.ExecuteCommandLine("pwd") + ")");
            }

            // Prepare communication channel
            Identifier inputLink = _agent.GetInputLink();

            if (inputLink == null)
                throw new Exception("Soar: Error getting the input link");

            Identifier overrideWME = _agent.CreateIdWME(inputLink, "override");
            _overrideActiveWME = _agent.CreateStringWME(overrideWME, "active", "false");
            Identifier drivePowerWME = _agent.CreateIdWME(overrideWME, "drive-power");
            _overrideLeftWME = _agent.CreateFloatWME(drivePowerWME, "left", 0);
            _overrideRightWME = _agent.CreateFloatWME(drivePowerWME, "right", 0);
            _overrideStopWME = _agent.CreateStringWME(drivePowerWME, "stop", "false");

            Identifier configWME = _agent.CreateIdWME(inputLink, "config");

            Identifier powerWME = _agent.CreateIdWME(configWME, "power");
            _agent.CreateFloatWME(powerWME, "drive", initialState.DrivePower);
            _agent.CreateFloatWME(powerWME, "reverse", initialState.ReversePower);

            Identifier delayWME = _agent.CreateIdWME(configWME, "delay");
            _agent.CreateFloatWME(delayWME, "stop", initialState.StopTimeout);
            _agent.CreateFloatWME(delayWME, "reverse", initialState.BackUpTimeout);
            _agent.CreateFloatWME(delayWME, "turn", initialState.TurnTimeout);
            _agent.CreateFloatWME(delayWME, "variance", initialState.TimeoutVariance);

            Identifier sensorsWME = _agent.CreateIdWME(inputLink, "sensors");
            Identifier bumperWME = _agent.CreateIdWME(sensorsWME, "bumper");
            Identifier frontWME = _agent.CreateIdWME(bumperWME, "front");
            _frontBumperPressedWME = _agent.CreateStringWME(frontWME, "pressed", "false");
            _frontBumperWasPressedWME = _agent.CreateStringWME(frontWME, "was-pressed", "false");
            Identifier rearWME = _agent.CreateIdWME(bumperWME, "rear");
            _rearBumperPressedWME = _agent.CreateStringWME(rearWME, "pressed", "false");
            _rearBumperWasPressedWME = _agent.CreateStringWME(rearWME, "was-pressed", "false");
            
            // Current time WME
            _timeWME = _agent.CreateFloatWME(inputLink, "time", 0);

            // Random number WME and supporting state
            _randomWME = _agent.CreateFloatWME(inputLink, "random", 0);
            if (initialState.HasRandomSeed)
            {
                _random = new Random(initialState.RandomSeed);
                Trace.WriteLine("Seeding Soar's random number generator.");
                _agent.ExecuteCommandLine("srand " + initialState.RandomSeed);
                if (_agent.HadError())
                {
                    throw new Exception("Failed to seed Soar's random number generator");
                }
            }
            else
            {
                _random = new Random();
            }

            // commit input link structure
            _agent.Commit();

            _updateCall = new sml.Kernel.UpdateEventCallback(UpdateEventCallback);
            _kernel.RegisterForUpdateEvent(sml.smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, _updateCall, null);

            // spawn debugger
            if (initialState.SpawnDebugger)
            {
                SpawnDebugger();
            }

            _drivePort = drivePort;

            _simulationStart = DateTime.Now;

            OnLog("Soar initialized.");
        }

        private void SpawnDebugger()
        {
            System.Diagnostics.Process debuggerProc = new System.Diagnostics.Process();
            debuggerProc.EnableRaisingEvents = false;
            debuggerProc.StartInfo.WorkingDirectory = "bin";
            debuggerProc.StartInfo.FileName = "java";
            debuggerProc.StartInfo.Arguments = "-jar SoarJavaDebugger.jar -remote";
            Trace.WriteLine("Spawning debugger in " + debuggerProc.StartInfo.WorkingDirectory);
            debuggerProc.Start();

            bool ready = false;
            // do this loop if timeout seconds is 0 (code for wait indefinitely) or if we have tries left
            for (int tries = 0; tries < 15; ++tries)
            {
                _kernel.GetAllConnectionInfo();
                if (_kernel.HasConnectionInfoChanged())
                {
                    for (int i = 0; i < _kernel.GetNumberConnections(); ++i)
                    {
                        ConnectionInfo info = _kernel.GetConnectionInfo(i);
                        if (info.GetName() == "java-debugger")
                        {
                            if (info.GetAgentStatus() == sml_Names.kStatusReady)
                            {
                                ready = true;
                                break;
                            }
                        }
                    }
                    if (ready)
                    {
                        break;
                    }
                }
                Trace.WriteLine("Waiting for java-debugger...");
                Thread.Sleep(1000);
            }

            if (!ready)
                OnLog("Debugger spawn failed!");
        }

        private void UpdateEventCallback(sml.smlUpdateEventId eventID, IntPtr callbackData, IntPtr kernelPtr, smlRunFlags runFlags)
        {
            // check for stop
            if (_stop)
            {
                _stop = false;
                OnLog("Soar: Update: Stopping all agents.");
                _kernel.StopAllAgents();
                return;
            }

            // read output link, cache commands
            int numberOfCommands = _agent.GetNumberCommands();
            Identifier command;
            bool receivedCommand = false;
            drive.SetDrivePowerRequest request = new drive.SetDrivePowerRequest();

            for (int i = 0; i < numberOfCommands; ++i)
            {
                command = _agent.GetCommand(i);
                String commandName = command.GetAttribute();

                switch (commandName)
                {
                    case "drive-power":
                        String leftPowerString = command.GetParameterValue("left");
                        if (leftPowerString != null)
                        {
                            receivedCommand = true;
                            request.LeftWheelPower = double.Parse(leftPowerString);
                        }

                        String rightPowerString = command.GetParameterValue("right");
                        if (rightPowerString != null)
                        {
                            receivedCommand = true;
                            request.RightWheelPower = double.Parse(rightPowerString);
                        }

                        String stopString = command.GetParameterValue("stop");
                        if (stopString != null)
                        {
                            if (bool.Parse(stopString))
                            {
                                receivedCommand = true;
                                request.LeftWheelPower = 0;
                                request.RightWheelPower = 0;
                                //TODO: StopNow();
                            }
                        }

                        if (receivedCommand)
                        {
                            command.AddStatusComplete();
                        }
                        else
                        {
                            OnLog("Soar: Unknown drive-power command.");
                            command.AddStatusError();
                        }

                        break;

                    case "stop-sim":
                        command.AddStatusComplete();
                        break;

                    default:
                        OnLog("Soar: Unknown command.");
                        command.AddStatusError();
                        break;
                }
            }

            if (receivedCommand && _drivePort != null)
            {
                _drivePort.SetDrivePower(request);
            }

            _agent.Update(_overrideActiveWME, Override.OverrideActive.ToString().ToLowerInvariant());

            OverrideState cachedOverrideState;
            lock (Override)
            {
                // cache state for input link
                cachedOverrideState = new OverrideState(Override);

                // unlock state
            }

            // write input link from cache
            _agent.Update(_overrideActiveWME, cachedOverrideState.OverrideActive.ToString().ToLowerInvariant());
            _agent.Update(_overrideLeftWME, cachedOverrideState.OverrideLeft);
            _agent.Update(_overrideRightWME, cachedOverrideState.OverrideRight);

            BumperState cachedBumperState;
            // lock state
            lock (Bumper)
            {
                // cache state
                cachedBumperState = new BumperState(Bumper);

                // reset flag
                Bumper.Reset();

                // unlock state
            }

            _agent.Update(_frontBumperPressedWME, cachedBumperState.FrontBumperPressed.ToString().ToLowerInvariant());
            _agent.Update(_rearBumperPressedWME, cachedBumperState.RearBumperPressed.ToString().ToLowerInvariant());

            _agent.Update(_frontBumperWasPressedWME, cachedBumperState.FrontBumperWasPressed.ToString().ToLowerInvariant());
            _agent.Update(_rearBumperWasPressedWME, cachedBumperState.RearBumperWasPressed.ToString().ToLowerInvariant());

            TimeSpan elapsed = System.DateTime.Now - _simulationStart;
            _agent.Update(_timeWME, elapsed.TotalMilliseconds);
            _agent.Update(_randomWME, _random.NextDouble());

            // commit input link changes
            _agent.Commit();
        }

        public void RunSoar()
        {
            _running = true;
            OnLog("Soar: Started.");
            _kernel.RunAllAgentsForever();
            _running = false;
            OnLog("Soar: Stopped.");
        }

        public void ShutdownSoar()
        {
            if (_kernel == null)
                return;

            while (_running)
            {
                OnLog("Soar: Stop requested.");
                _stop = true;
                System.Threading.Thread.Sleep(100);
            }

            _kernel.Shutdown();
            _kernel = null;
        }
    }

}
