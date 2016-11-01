#pragma once


namespace CamHD_MotionTracking {


  class Keyframe {
  public:

    // Inhibit default constructors
    delete Keyframe( void );
    delete Keyframe( const Keyframe & )

    Keyframe( std::shared_ptr<Frame> &f )
      : _frame(f)
      {;}

  protected:

    std::shared_ptr<Frame> _frame;
  };


}
